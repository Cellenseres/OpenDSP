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

static int cmp(const void *a, const void *b) { return strcasecmp(*(char *const *)a, *(char *const *)b); }

int wav_scan(void)
{
    for (int i = 0; i < nfiles; i++) free(names[i]);
    nfiles = 0;
    mkdir("sdmc:/3ds", 0777);
    mkdir(WAV_DIR, 0777);
    DIR *d = opendir(WAV_DIR);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) && nfiles < MAX_FILES) {
        size_t l = strlen(e->d_name);
        if (l > 4 && (!strcasecmp(e->d_name + l - 4, ".wav") || !strcasecmp(e->d_name + l - 4, ".aac"))) {
            char *name = strdup(e->d_name);
            if (name) names[nfiles++] = name;
        }
    }
    closedir(d);
    qsort(names, nfiles, sizeof names[0], cmp);
    return nfiles;
}
int wav_count(void) { return nfiles; }
const char *wav_name(int i) { return i >= 0 && i < nfiles ? names[i] : ""; }

void wav_frame_notify(void) { opendspAudioNotifyFrame(); }

void wav_apply(void)
{
    if (!stream) return;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    if (R_FAILED(opendspAudioGetInfo(stream, &fi))) return;
    float rate = fi.sample_rate * (S.speed / 100.f);
    if (rate > 4 * 32728.f) rate = 4 * 32728.f;
    opendspAudioSetRate(stream, rate);
    ndspChnSetInterp(CH_WAV, S.interp);
    float mix[12];
    audio_build_mix(CH_WAV, 1, 1, mix);
    opendspAudioSetMix(stream, mix);
}

static bool close_stream(void)
{
    Result r = opendspAudioClose(&stream);
    if (R_FAILED(r)) snprintf(stream_error, sizeof(stream_error), "Player busy: %08lX", (unsigned long)r);
    return R_SUCCEEDED(r);
}

bool wav_play(int i, char *err, size_t errlen)
{
    if (i < 0 || i >= nfiles) {
        snprintf(err, errlen, "No file selected"); return false;
    }
    if (!close_stream()) {
        snprintf(err, errlen, "%s", stream_error); return false;
    }
    stream_error[0] = 0;
    Result r = opendspAudioInit(1u << CH_WAV);
    if (R_FAILED(r)) goto fail;
    char path[300];
    if (snprintf(path, sizeof(path), "%s/%s", WAV_DIR, names[i]) >= (int)sizeof(path)) {
        r = OPENDSP_AUDIO_ERR_ARGUMENT; goto fail;
    }
    opendspAudioConfig cfg = OPENDSP_AUDIO_CONFIG_INIT;
    cfg.channel = CH_WAV;
    cfg.aac_backend = requested_backend;
    cfg.flags = looping ? OPENDSP_AUDIO_LOOP : 0;
    r = opendspAudioOpen(&stream, path, &cfg);
    if (R_FAILED(r)) goto fail;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    opendspAudioGetInfo(stream, &fi);
    snprintf(info, sizeof(info), "%lu Hz %s %s", (unsigned long)fi.sample_rate,
        fi.channels == 2 ? "stereo" : "mono", fi.format == OPENDSP_AUDIO_AAC ?
        (opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP ? "AAC / Teak DSP" : "AAC / ARM11") :
        fi.bits == 16 ? "PCM16" : "PCM8");
    wav_apply();
    r = opendspAudioPlay(stream);
    if (R_SUCCEEDED(r)) return true;
fail:
    snprintf(stream_error, sizeof(stream_error), "Player: %08lX", (unsigned long)r);
    snprintf(err, errlen, "%s", stream_error);
    close_stream();
    return false;
}

void wav_stop(void) { if (stream) opendspAudioStop(stream); }
bool wav_playing(void)
{
    if (!stream) return false;
    opendspAudioState state = opendspAudioGetState(stream);
    return state == OPENDSP_AUDIO_BUFFERING || state == OPENDSP_AUDIO_PLAYING ||
        state == OPENDSP_AUDIO_PAUSED || state == OPENDSP_AUDIO_DRAINING;
}
void wav_set_loop(bool loop) { looping = loop; if (stream) opendspAudioSetLoop(stream, loop); }
bool wav_loop(void) { return looping; }
const char *wav_info(void) { return wav_playing() ? info : ""; }
unsigned wav_backend(void) { return requested_backend; }
unsigned wav_active_backend(void) { return stream ? opendspAudioGetBackend(stream) : OPENDSP_AAC_BACKEND_AUTO; }
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
    if (backend != OPENDSP_AAC_BACKEND_ARM11 && backend != OPENDSP_AAC_BACKEND_DSP) return;
    if (!close_stream()) return;
    requested_backend = backend;
    stream_error[0] = 0;
}
float wav_decode_load(void)
{
    if (!wav_playing() || wav_active_backend() != OPENDSP_AAC_BACKEND_ARM11) return -1;
    opendspAudioStats st = {.struct_size = sizeof(st)};
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    opendspAudioGetStats(stream, &st);
    opendspAudioGetInfo(stream, &fi);
    return st.decoded_frames ? (float)st.decode_ticks * fi.sample_rate / (SYSCLOCK_ARM11 * (float)st.decoded_frames) : -1;
}
float wav_progress(void)
{
    if (!stream) return -1;
    opendspAudioInfo fi = {.struct_size = sizeof(fi)};
    opendspAudioGetInfo(stream, &fi);
    return fi.duration_known && fi.total_frames ? (float)opendspAudioGetPosition(stream) / fi.total_frames : -1;
}
bool wav_stats(opendspAudioStats *stats) { return stream && R_SUCCEEDED(opendspAudioGetStats(stream, stats)); }
void wav_exit(void)
{
    close_stream();
    opendspAudioExit();
    for (int i = 0; i < nfiles; ++i) free(names[i]);
    nfiles = 0;
}
