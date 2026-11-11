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
static unsigned requested_backend = OPENDSP_AAC_BACKEND_AUTO;
static char info[64], stream_error[80];
static char playing_name[256];
static char diagnostic_name[256];
static unsigned diagnostic_backend;

void opendsp_codec_diagnostic(const char *stage, Result result)
{
    FILE *log = fopen(WAV_DIR "/dsp-errors.log", "a");
    if (!log) return;
    if (!fseek(log, 0, SEEK_END) && ftell(log) < 65536)
        fprintf(log, "%s | %s | backend=%u | %08lX\n", diagnostic_name, stage,
                diagnostic_backend, (unsigned long)(u32)result);
    fclose(log);
}

static bool pcm_format(opendspAudioFormat format)
{
    return format == OPENDSP_AUDIO_WAV || format == OPENDSP_AUDIO_MSU1;
}

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
        const char *extension = strrchr(e->d_name, '.');
        if (l > 4 && extension &&
            (!strcasecmp(extension, ".wav") || !strcasecmp(extension, ".aac") ||
             !strcasecmp(extension, ".mp3") || !strcasecmp(extension, ".ogg") ||
             !strcasecmp(extension, ".oga") || !strcasecmp(extension, ".opus") ||
             !strcasecmp(extension, ".pcm") || !strcasecmp(extension, ".opuz"))) {
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
    snprintf(diagnostic_name, sizeof(diagnostic_name), "%s", names[i]);
    diagnostic_backend = backend;
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
    const char *extension = strrchr(names[i], '.');
    if (extension && !strcasecmp(extension, ".pcm"))
        cfg.flags |= OPENDSP_AUDIO_MSU1_PCM;
    r = opendspAudioOpen(&stream, path, &cfg);
    if (R_FAILED(r))
        goto fail;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    r = opendspAudioGetInfo(stream, &fi);
    if (R_FAILED(r))
        goto fail;
    if (!pcm_format(fi.format) &&
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
        : fi.format == OPENDSP_AUDIO_VORBIS
            ? (opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP ? "VORBIS / Teak DSP" : "VORBIS / ARM11")
        : fi.format == OPENDSP_AUDIO_OPUS
            ? (opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP ? "OPUS / Teak DSP" : "OPUS / ARM11")
        : fi.format == OPENDSP_AUDIO_OPUZ
            ? (opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP ? "OPUZ / Teak DSP" : "OPUZ / ARM11")
        : fi.format == OPENDSP_AUDIO_MSU1 ? "MSU-1 PCM16"
        : fi.bits == 16 ? "PCM16"
                        : "PCM8");
    wav_apply();
    r = opendspAudioPlay(stream);
    if (R_SUCCEEDED(r)) {
        snprintf(playing_name, sizeof(playing_name), "%s", names[i]);
        return true;
    }
fail:
    opendsp_codec_diagnostic("Player open", r);
    if (r == OPENDSP_AAC_ERR_BUSY)
        snprintf(stream_error, sizeof(stream_error), "DSP reserved: restart Sound Lab (%08lX)", (unsigned long)r);
    else if (r == OPENDSP_AUDIO_ERR_UNSUPPORTED)
        snprintf(stream_error, sizeof(stream_error), "Decoder unsupported (%08lX)", (unsigned long)r);
    else
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
bool wav_pause_resume(char *error, size_t length)
{
    Result r = OPENDSP_AUDIO_ERR_STATE;
    if (stream) {
        if (opendspAudioGetState(stream) == OPENDSP_AUDIO_PAUSED)
            r = opendspAudioResume(stream);
        else
            r = opendspAudioPause(stream);
    }
    if (R_FAILED(r))
        snprintf(error, length, "Pause/resume: %08lX", (unsigned long)r);
    return R_SUCCEEDED(r);
}
bool wav_seek_middle(char *error, size_t length)
{
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    if (!wav_stream_info(&fi)) {
        snprintf(error, length, "Play a track first");
        return false;
    }
    if (fi.format == OPENDSP_AUDIO_AAC || fi.format == OPENDSP_AUDIO_MP3) {
        snprintf(error, length, "AAC/MP3 seek supports rewind only");
        return false;
    }
    if (!fi.duration_known || !fi.total_frames) {
        snprintf(error, length, "Track duration unavailable");
        return false;
    }
    Result r = opendspAudioSeek(stream, fi.total_frames / 2);
    if (R_FAILED(r))
        snprintf(error, length, "Seek: %08lX", (unsigned long)r);
    return R_SUCCEEDED(r);
}
u64 wav_position(void)
{
    return stream ? opendspAudioGetPosition(stream) : 0;
}
bool wav_loop_info(opendspAudioLoopInfo *out)
{
    return stream && R_SUCCEEDED(opendspAudioGetLoopInfo(stream, out));
}
bool wav_playing(void)
{
    if (!stream)
        return false;
    opendspAudioState state = opendspAudioGetState(stream);
    return state == OPENDSP_AUDIO_BUFFERING || state == OPENDSP_AUDIO_PLAYING ||
           state == OPENDSP_AUDIO_PAUSED || state == OPENDSP_AUDIO_DRAINING;
}
bool wav_set_loop(bool loop, char *error, size_t length)
{
    if (stream) {
        Result r = opendspAudioSetLoop(stream, loop);
        if (R_FAILED(r)) {
            opendspAudioLoopInfo info = {.struct_size = sizeof(info)};
            if (loop && wav_loop_info(&info) && info.status == OPENDSP_AUDIO_LOOP_INVALID)
                snprintf(error, length, "Invalid loop tags; looping stayed off");
            else
                snprintf(error, length, "Loop: %08lX", (unsigned long)r);
            return false;
        }
    }
    looping = loop;
    return true;
}
bool wav_loop(void)
{
    return looping;
}
const char *wav_state_name(opendspAudioStream *s)
{
    if (!s)
        return "Stopped";
    switch (opendspAudioGetState(s)) {
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
const char *wav_status(void)
{
    return wav_state_name(stream);
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
bool wav_set_backend(unsigned backend, char *error, size_t length)
{
    if (backend != OPENDSP_AAC_BACKEND_ARM11 && backend != OPENDSP_AAC_BACKEND_DSP) {
        snprintf(error, length, "Invalid decoder selection");
        return false;
    }
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    if (wav_playing() && wav_stream_info(&fi) && !pcm_format(fi.format))
        return wav_select_backend(backend, error, length);
    requested_backend = backend;
    stream_error[0] = 0;
    return true;
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
bool wav_decoder_stats(opendspAudioDecoderStats *stats)
{
    return stream && R_SUCCEEDED(opendspAudioGetDecoderStats(stream, stats));
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
    if (!wav_stream_info(&fi) || pcm_format(fi.format))
        return wav_set_backend(backend, error, length);
    if (!opendspAudioSupportsBackend(fi.format, backend)) {
        snprintf(error, length, "%s decoder is unavailable",
                 backend == OPENDSP_AAC_BACKEND_DSP ? "DSP" : "ARM11");
        return false;
    }
    if (opendspAudioGetBackend(stream) == backend) {
        requested_backend = backend;
        return true;
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
        snprintf(error, length, "Play a compressed track first");
        return false;
    }
    return wav_select_backend(active == OPENDSP_AAC_BACKEND_DSP ? OPENDSP_AAC_BACKEND_ARM11
                                                                : OPENDSP_AAC_BACKEND_DSP,
                              error, length);
}
bool wav_release(void)
{
    return close_stream();
}
void wav_exit(void)
{
    close_stream();
    opendspAudioExit();
    for (int i = 0; i < nfiles; ++i)
        free(names[i]);
    nfiles = 0;
}
