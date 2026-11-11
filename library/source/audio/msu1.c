// SPDX-License-Identifier: BSD-2-Clause
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <opendsp/msu1.h>

#define MSU1_REVISION 2
enum { STATUS_REPEAT = 0x20, STATUS_PLAY = 0x10, STATUS_ERROR = 0x08 };

struct opendspMsu1 {
    char *prefix, *path;
    size_t path_bytes;
    s32 channel;
    opendspAudioStream *stream, *retired;
    FILE *data;
    u32 data_seek;
    u16 track_latch, track, resume_track;
    u64 resume_frame;
    u8 volume, kind;
    bool loaded, error, play, repeat, resume, started, paused, has_mix;
    float mix[12];
};

// Tried in order, starting with the last that loaded.
static const char *const kinds[] = {".pcm", ".opuz", ".opus", ".ogg"};
#define KINDS (sizeof(kinds) / sizeof(kinds[0]))

static bool track_file(const char *name, size_t *base)
{
    size_t length = strlen(name), extension = 0;
    for (unsigned k = 0; k < KINDS && !extension; ++k) {
        size_t bytes = strlen(kinds[k]);
        if (length > bytes && !strcasecmp(name + length - bytes, kinds[k]))
            extension = bytes;
    }
    if (!extension)
        return false;
    size_t i = length - extension;
    while (i && name[i - 1] >= '0' && name[i - 1] <= '9')
        --i;
    if (i == length - extension || i < 2 || name[i - 1] != '-')
        return false;
    *base = i - 1;
    return true;
}

static char *copy_text(const char *text, size_t length)
{
    char *copy = malloc(length + 1);
    if (copy) {
        memcpy(copy, text, length);
        copy[length] = 0;
    }
    return copy;
}

#define PACK_BASES 8

// Picks the .msu name, else the most common track base.
static char *find_pack(const char *directory)
{
    DIR *d = opendir(directory);
    if (!d)
        return NULL;
    char *bases[PACK_BASES] = {0}, *data = NULL;
    unsigned counts[PACK_BASES] = {0}, used = 0;
    struct dirent *e;
    while (!data && (e = readdir(d))) {
        size_t length = strlen(e->d_name), base;
        if (length > 4 && !strcasecmp(e->d_name + length - 4, ".msu")) {
            data = copy_text(e->d_name, length - 4);
        } else if (track_file(e->d_name, &base)) {
            unsigned i = 0;
            while (i < used && (strlen(bases[i]) != base || strncmp(bases[i], e->d_name, base)))
                ++i;
            if (i == used && used < PACK_BASES && (bases[i] = copy_text(e->d_name, base)))
                ++used;
            if (i < used)
                ++counts[i];
        }
    }
    closedir(d);
    unsigned best = 0;
    for (unsigned i = 1; i < used; ++i)
        if (counts[i] > counts[best])
            best = i;
    const char *name = data ? data : used ? bases[best] : NULL;
    char *prefix = NULL;
    if (name) {
        size_t end = strlen(directory);
        while (end > 1 && directory[end - 1] == '/')
            --end;
        size_t bytes = end + strlen(name) + 2;
        prefix = malloc(bytes);
        if (prefix)
            snprintf(prefix, bytes, "%.*s/%s", (int)end, directory, name);
    }
    free(data);
    for (unsigned i = 0; i < used; ++i)
        free(bases[i]);
    return prefix;
}

static void retry_retired(opendspMsu1 *m)
{
    if (m->retired)
        opendspAudioClose(&m->retired);
}

static bool release(opendspMsu1 *m)
{
    retry_retired(m);
    if (!m->stream)
        return true;
    opendspAudioStop(m->stream);
    if (R_SUCCEEDED(opendspAudioClose(&m->stream)))
        return true;
    if (m->retired)
        return false;
    m->retired = m->stream;
    m->stream = NULL;
    return true;
}

static void load_track(opendspMsu1 *m)
{
    m->loaded = m->play = m->repeat = m->started = m->paused = false;
    m->error = true;
    if (!release(m))
        return;
    opendspAudioConfig config = OPENDSP_AUDIO_CONFIG_INIT;
    config.channel = m->channel;
    bool opened = false;
    for (unsigned i = 0; i < KINDS && !opened; ++i) {
        unsigned kind = (m->kind + i) % KINDS;
        snprintf(m->path, m->path_bytes, "%s-%u%s", m->prefix, (unsigned)m->track, kinds[kind]);
        config.flags = kind ? 0 : OPENDSP_AUDIO_MSU1_PCM;
        opened = R_SUCCEEDED(opendspAudioOpen(&m->stream, m->path, &config));
        if (opened)
            m->kind = (u8)kind;
    }
    if (!opened)
        return;
    if (m->has_mix)
        opendspAudioSetMix(m->stream, m->mix);
    opendspAudioSetVolume(m->stream, m->volume / 255.f);
    // Bit 2 of $2007 bookmarked this track.
    if (m->resume && m->resume_track == m->track) {
        opendspAudioSeek(m->stream, m->resume_frame);
        m->resume = false;
    }
    m->loaded = true;
    m->error = false;
}

void opendspMsu1Reset(opendspMsu1 *m)
{
    if (!m)
        return;
    release(m);
    m->loaded = m->error = m->play = m->repeat = m->resume = m->started = m->paused = false;
    m->track_latch = m->track = 0;
    m->volume = 0;
    m->data_seek = 0;
    if (m->data)
        fseek(m->data, 0, SEEK_SET);
}

Result opendspMsu1Open(opendspMsu1 **out, const char *pack, const opendspMsu1Config *config)
{
    if (!out)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    *out = NULL;
    opendspMsu1Config cfg = OPENDSP_MSU1_CONFIG_INIT;
    if (config) {
        if (config->struct_size < sizeof(cfg))
            return OPENDSP_AUDIO_ERR_ARGUMENT;
        cfg = *config;
    }
    if (!pack || !*pack || cfg.channel < -1 || cfg.channel >= 24)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    opendspMsu1 *m = calloc(1, sizeof(*m));
    if (!m)
        return OPENDSP_AUDIO_ERR_MEMORY;
    m->channel = cfg.channel;
    struct stat info;
    if (!stat(pack, &info) && S_ISDIR(info.st_mode)) {
        m->prefix = find_pack(pack);
        if (!m->prefix) {
            free(m);
            return OPENDSP_AUDIO_ERR_IO;
        }
    } else {
        size_t length = strlen(pack);
        if (length > 4 && !strcasecmp(pack + length - 4, ".msu"))
            length -= 4;
        m->prefix = copy_text(pack, length);
    }
    m->path_bytes = m->prefix ? strlen(m->prefix) + 16 : 0;
    m->path = m->prefix ? malloc(m->path_bytes) : NULL;
    if (!m->path) {
        free(m->prefix);
        free(m);
        return OPENDSP_AUDIO_ERR_MEMORY;
    }
    snprintf(m->path, m->path_bytes, "%s.msu", m->prefix);
    m->data = fopen(m->path, "rb");
    *out = m;
    return 0;
}

Result opendspMsu1Close(opendspMsu1 **msu)
{
    if (!msu || !*msu)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    opendspMsu1 *m = *msu;
    m->loaded = m->play = false;
    if (!release(m) || m->stream || m->retired)
        return OPENDSP_AUDIO_ERR_BUSY;
    if (m->data)
        fclose(m->data);
    free(m->path);
    free(m->prefix);
    free(m);
    *msu = NULL;
    return 0;
}

static bool finished(opendspMsu1 *m)
{
    opendspAudioState state = opendspAudioGetState(m->stream);
    return state == OPENDSP_AUDIO_FINISHED || state == OPENDSP_AUDIO_ERROR;
}

u8 opendspMsu1Read(opendspMsu1 *m, u32 address)
{
    static const char id[] = "S-MSU1";
    if (!m)
        return 0;
    address &= 7;
    if (address >= 2)
        return (u8)id[address - 2];
    if (address == 1) {
        int byte = m->data ? fgetc(m->data) : EOF;
        return byte == EOF ? 0 : (u8)byte;
    }
    retry_retired(m);
    if (m->play && (!m->stream || finished(m)))
        m->play = false;
    return MSU1_REVISION | (m->error ? STATUS_ERROR : 0) | (m->repeat ? STATUS_REPEAT : 0) |
           (m->play ? STATUS_PLAY : 0);
}

static void control(opendspMsu1 *m, u8 value)
{
    bool play = value & 1, repeat = (value & 2) != 0;
    if (!play && (value & 4)) {
        m->resume = true;
        m->resume_track = m->track;
        m->resume_frame = opendspAudioGetPosition(m->stream);
    }
    if (repeat != m->repeat)
        opendspAudioSetLoop(m->stream, repeat);
    m->repeat = repeat;
    if (!play) {
        if (m->play && R_SUCCEEDED(opendspAudioPause(m->stream)))
            m->paused = true;
        m->play = false;
        return;
    }
    // A finished track stays silent, as on hardware.
    if (finished(m))
        return;
    Result r = 0;
    if (!m->started)
        r = opendspAudioPlay(m->stream);
    else if (m->paused)
        r = opendspAudioResume(m->stream);
    m->started = m->started || R_SUCCEEDED(r);
    m->paused = m->paused && R_FAILED(r);
    m->play = R_SUCCEEDED(r);
}

void opendspMsu1Write(opendspMsu1 *m, u32 address, u8 value)
{
    if (!m)
        return;
    switch (address & 7) {
    case 0:
    case 1:
    case 2:
    case 3: {
        unsigned shift = (address & 3) * 8;
        m->data_seek = (m->data_seek & ~(0xffu << shift)) | (u32)value << shift;
        if ((address & 3) == 3 && m->data)
            fseek(m->data, m->data_seek, SEEK_SET);
        break;
    }
    case 4:
        m->track_latch = (m->track_latch & 0xff00) | value;
        break;
    case 5:
        m->track_latch = (m->track_latch & 0x00ff) | (u16)value << 8;
        m->track = m->track_latch;
        load_track(m);
        break;
    case 6:
        m->volume = value;
        if (m->stream)
            opendspAudioSetVolume(m->stream, value / 255.f);
        break;
    case 7:
        // Ignored while busy or after a load error.
        if (m->loaded && m->stream)
            control(m, value);
        break;
    }
}

Result opendspMsu1SetMix(opendspMsu1 *m, const float mix[12])
{
    if (!m || !mix)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    memcpy(m->mix, mix, sizeof(m->mix));
    m->has_mix = true;
    return m->stream ? opendspAudioSetMix(m->stream, m->mix) : 0;
}

opendspAudioStream *opendspMsu1GetStream(opendspMsu1 *m)
{
    return m ? m->stream : NULL;
}

const char *opendspMsu1GetPack(const opendspMsu1 *m)
{
    return m ? m->prefix : NULL;
}
