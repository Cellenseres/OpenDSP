// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <opendsp/audio.h>
#include "wav.h"
#include "audio.h"

#define MAX_FILES 64
static char *names[MAX_FILES];
static int nfiles;
static opendspAudioStream *stream;
static bool looping;
static unsigned requested_backend = OPENDSP_AAC_BACKEND_DSP;
static char info[64], stream_error[80];
static char playing_name[256];

static int cmp(const void *a, const void *b)
{
    return strcasecmp(*(char *const *)a, *(char *const *)b);
}

int wav_scan(void)
{
    for (int i = 0; i < nfiles; i++)
        free(names[i]);
    nfiles = 0;
    mkdir("sdmc:/3ds", 0777);
    mkdir(WAV_DIR, 0777);
    DIR *d = opendir(WAV_DIR);
    if (!d)
        return 0;
    struct dirent *e;
    while ((e = readdir(d)) && nfiles < MAX_FILES) {
        size_t l = strlen(e->d_name);
        if (l > 4 && (!strcasecmp(e->d_name + l - 4, ".wav") || !strcasecmp(e->d_name + l - 4, ".aac") ||
                      !strcasecmp(e->d_name + l - 4, ".mp3"))) {
            char *name = strdup(e->d_name);
            if (name)
                names[nfiles++] = name;
        }
    }
    closedir(d);
    qsort(names, nfiles, sizeof names[0], cmp);
    return nfiles;
}
int wav_count(void)
{
    return nfiles;
}
const char *wav_name(int i)
{
    return i >= 0 && i < nfiles ? names[i] : "";
}

void wav_frame_notify(void)
{
    opendspAudioNotifyFrame();
}

void wav_apply(void)
{
    if (!stream)
        return;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    if (R_FAILED(opendspAudioGetInfo(stream, &fi)))
        return;
    float rate = fi.sample_rate * (S.speed / 100.f);
    if (rate > 4 * 32728.f)
        rate = 4 * 32728.f;
    opendspAudioSetRate(stream, rate);
    ndspChnSetInterp(CH_WAV, S.interp);
    float mix[12];
    audio_build_mix(CH_WAV, 1, 1, mix);
    opendspAudioSetMix(stream, mix);
}

static bool close_stream(void)
{
    Result r = opendspAudioClose(&stream);
    if (R_FAILED(r))
        snprintf(stream_error, sizeof(stream_error), "Player busy: %08lX", (unsigned long)r);
    return R_SUCCEEDED(r);
}

static bool play_backend(int i, unsigned backend, char *err, size_t errlen)
{
    if (i < 0 || i >= nfiles) {
        snprintf(err, errlen, "No file selected");
        return false;
    }
    if (!close_stream()) {
        snprintf(err, errlen, "%s", stream_error);
        return false;
    }
    stream_error[0] = 0;
    Result r = opendspAudioInit(1u << CH_WAV);
    if (R_FAILED(r))
        goto fail;
    char path[300];
    if (snprintf(path, sizeof(path), "%s/%s", WAV_DIR, names[i]) >= (int)sizeof(path)) {
        r = OPENDSP_AUDIO_ERR_ARGUMENT;
        goto fail;
    }
    opendspAudioConfig cfg = OPENDSP_AUDIO_CONFIG_INIT;
    cfg.channel = CH_WAV;
    cfg.aac_backend = backend;
    cfg.flags = looping ? OPENDSP_AUDIO_LOOP : 0;
    r = opendspAudioOpen(&stream, path, &cfg);
    if (R_FAILED(r))
        goto fail;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    r = opendspAudioGetInfo(stream, &fi);
    if (R_FAILED(r))
        goto fail;
    if ((fi.format == OPENDSP_AUDIO_MP3 || fi.format == OPENDSP_AUDIO_AAC) &&
        backend != OPENDSP_AAC_BACKEND_AUTO && opendspAudioGetBackend(stream) != backend) {
        snprintf(stream_error, sizeof(stream_error), "Decoder unavailable (%u / %lu)", backend,
                 (unsigned long)opendspAudioGetBackend(stream));
        snprintf(err, errlen, "%s", stream_error);
        close_stream();
        return false;
    }
    snprintf(
        info, sizeof(info), "%lu Hz %s %s", (unsigned long)fi.sample_rate,
        fi.channels == 2 ? "stereo" : "mono",
        fi.format == OPENDSP_AUDIO_MP3
            ? (opendspAudioGetBackend(stream) == OPENDSP_MP3_BACKEND_DSP ? "MP3 / Teak DSP" : "MP3 / ARM11")
        : fi.format == OPENDSP_AUDIO_AAC
            ? (opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP ? "AAC / Teak DSP" : "AAC / ARM11")
        : fi.bits == 16 ? "PCM16"
                        : "PCM8");
    wav_apply();
    r = opendspAudioPlay(stream);
    if (R_SUCCEEDED(r)) {
        snprintf(playing_name, sizeof(playing_name), "%s", names[i]);
        return true;
    }
fail:
    snprintf(stream_error, sizeof(stream_error), "Player: %08lX", (unsigned long)r);
    snprintf(err, errlen, "%s", stream_error);
    close_stream();
    return false;
}
bool wav_play(int i, char *err, size_t errlen)
{
    return play_backend(i, requested_backend, err, errlen);
}

void wav_stop(void)
{
    if (stream)
        opendspAudioStop(stream);
}
bool wav_playing(void)
{
    if (!stream)
        return false;
    opendspAudioState state = opendspAudioGetState(stream);
    return state == OPENDSP_AUDIO_BUFFERING || state == OPENDSP_AUDIO_PLAYING ||
           state == OPENDSP_AUDIO_PAUSED || state == OPENDSP_AUDIO_DRAINING;
}
void wav_set_loop(bool loop)
{
    looping = loop;
    if (stream)
        opendspAudioSetLoop(stream, loop);
}
bool wav_loop(void)
{
    return looping;
}
const char *wav_status(void)
{
    if (!stream)
        return "Stopped";
    switch (opendspAudioGetState(stream)) {
    case OPENDSP_AUDIO_BUFFERING:
        return "Buffering";
    case OPENDSP_AUDIO_PLAYING:
        return "Playing";
    case OPENDSP_AUDIO_PAUSED:
        return "Paused";
    case OPENDSP_AUDIO_DRAINING:
        return "Playing";
    case OPENDSP_AUDIO_FINISHED:
        return "Finished";
    case OPENDSP_AUDIO_ERROR:
        return "Error";
    default:
        return "Stopped";
    }
}
const char *wav_info(void)
{
    return wav_playing() ? info : "";
}
unsigned wav_backend(void)
{
    return requested_backend;
}
unsigned wav_active_backend(void)
{
    return stream ? opendspAudioGetBackend(stream) : OPENDSP_AAC_BACKEND_AUTO;
}
const char *wav_error(void)
{
    if (stream) {
        Result r = opendspAudioGetLastError(stream);
        if (r == OPENDSP_AUDIO_ERR_CONTEXT)
            snprintf(stream_error, sizeof(stream_error), "DSP resumed: restart the track");
        else if (R_FAILED(r))
            snprintf(stream_error, sizeof(stream_error), "Playback error: %08lX", (unsigned long)r);
    }
    return stream_error;
}
void wav_set_backend(unsigned backend)
{
    if (backend != OPENDSP_AAC_BACKEND_ARM11 && backend != OPENDSP_AAC_BACKEND_DSP)
        return;
    if (!close_stream())
        return;
    requested_backend = backend;
    stream_error[0] = 0;
}
float wav_progress(void)
{
    if (!stream)
        return -1;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    opendspAudioGetInfo(stream, &fi);
    return fi.duration_known && fi.total_frames ? (float)opendspAudioGetPosition(stream) / fi.total_frames
                                                : -1;
}
bool wav_stats(opendspAudioStats *stats)
{
    return stream && R_SUCCEEDED(opendspAudioGetStats(stream, stats));
}
bool wav_stream_info(opendspAudioInfo *out)
{
    return stream && R_SUCCEEDED(opendspAudioGetInfo(stream, out));
}
bool wav_select_backend(unsigned backend, char *error, size_t length)
{
    if (length)
        error[0] = 0;
    if (backend != OPENDSP_AAC_BACKEND_ARM11 && backend != OPENDSP_AAC_BACKEND_DSP) {
        snprintf(error, length, "Invalid decoder selection");
        return false;
    }
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    if (!wav_stream_info(&fi) || (fi.format != OPENDSP_AUDIO_AAC && fi.format != OPENDSP_AUDIO_MP3)) {
        snprintf(error, length, "Play an AAC or MP3 track first");
        return false;
    }
    for (int i = 0; i < nfiles; ++i)
        if (!strcmp(names[i], playing_name)) {
            if (play_backend(i, backend, error, length)) {
                requested_backend = backend;
                return true;
            }
            return false;
        }
    snprintf(error, length, "Track missing; rescan SD card");
    return false;
}
bool wav_compare_backend(char *error, size_t length)
{
    unsigned active = wav_active_backend();
    if (active != OPENDSP_AAC_BACKEND_DSP && active != OPENDSP_AAC_BACKEND_ARM11) {
        snprintf(error, length, "Play an AAC or MP3 track first");
        return false;
    }
    return wav_select_backend(active == OPENDSP_AAC_BACKEND_DSP ? OPENDSP_AAC_BACKEND_ARM11
                                                                : OPENDSP_AAC_BACKEND_DSP,
                              error, length);
}
void wav_exit(void)
{
    close_stream();
    opendspAudioExit();
    for (int i = 0; i < nfiles; ++i)
        free(names[i]);
    nfiles = 0;
}
