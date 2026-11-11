// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <opendsp/msu1.h>
#include "audio.h"
#include "msu.h"

#define MAX_TRACKS 256

static opendspMsu1 *msu;
static u16 tracks[MAX_TRACKS];
static int track_count, track;
static int volume = 255, fade_volume = -1;
static bool repeat;

static void reg_write(u32 address, u8 value)
{
    opendspMsu1Write(msu, address, value);
}

static int compare(const void *a, const void *b)
{
    return (int)*(const u16 *)a - (int)*(const u16 *)b;
}

// Lists <pack>-<N>.pcm numbers for the track picker.
static void scan_tracks(void)
{
    track_count = 0;
    const char *pack = strrchr(opendspMsu1GetPack(msu), '/');
    pack = pack ? pack + 1 : opendspMsu1GetPack(msu);
    size_t length = strlen(pack);
    DIR *d = opendir(MSU_DIR);
    struct dirent *e;
    while (d && (e = readdir(d)) && track_count < MAX_TRACKS) {
        const char *name = e->d_name;
        if (strncmp(name, pack, length) || name[length] != '-')
            continue;
        char *end;
        unsigned long number = strtoul(name + length + 1, &end, 10);
        bool track = !strcasecmp(end, ".pcm") || !strcasecmp(end, ".opuz") || !strcasecmp(end, ".opus") ||
                     !strcasecmp(end, ".ogg");
        if (end != name + length + 1 && track && number <= 0xffff && !msu_track_present((int)number))
            tracks[track_count++] = (u16)number;
    }
    if (d)
        closedir(d);
    qsort(tracks, track_count, sizeof(tracks[0]), compare);
    track = track_count ? tracks[0] : 0;
}

bool msu_open(char *error, size_t length)
{
    msu_close();
    if (!wav_release()) {
        snprintf(error, length, "File player busy");
        return false;
    }
    Result r = opendspAudioInit(1u << CH_WAV);
    opendspMsu1Config config = OPENDSP_MSU1_CONFIG_INIT;
    config.channel = CH_WAV;
    if (R_SUCCEEDED(r))
        r = opendspMsu1Open(&msu, MSU_DIR, &config);
    if (R_FAILED(r)) {
        if (r != OPENDSP_AUDIO_ERR_IO)
            snprintf(error, length, "MSU-1: %08lX", (unsigned long)r);
        return false;
    }
    scan_tracks();
    msu_apply();
    reg_write(0x2006, (u8)volume);
    repeat = false;
    fade_volume = -1;
    return true;
}

void msu_close(void)
{
    if (msu)
        opendspMsu1Close(&msu);
}

bool msu_ready(void)
{
    return msu != NULL;
}

const char *msu_pack(void)
{
    const char *pack = msu ? strrchr(opendspMsu1GetPack(msu), '/') : NULL;
    return pack ? pack + 1 : "";
}

int msu_track(void)
{
    return track;
}

bool msu_track_present(int number)
{
    for (int i = 0; i < track_count; ++i)
        if (tracks[i] == number)
            return true;
    return false;
}

void msu_step_track(int delta, bool any)
{
    if (any || !track_count) {
        track = (track + delta) & 0xffff;
        return;
    }
    int i = 0;
    while (i < track_count && tracks[i] < track)
        ++i;
    if (delta > 0)
        i = i < track_count && tracks[i] == track ? i + 1 : i;
    else
        i = i - 1;
    track = tracks[(i % track_count + track_count) % track_count];
}

void msu_load(void)
{
    if (!msu)
        return;
    fade_volume = -1;
    reg_write(0x2006, (u8)volume);
    reg_write(0x2004, (u8)track);
    reg_write(0x2005, (u8)(track >> 8));
    msu_apply();
    reg_write(0x2007, 1 | (repeat ? 2 : 0));
}

static void control(bool play, bool bookmark)
{
    reg_write(0x2007, (play ? 1 : 0) | (repeat ? 2 : 0) | (bookmark ? 4 : 0));
}

void msu_play_pause(void)
{
    if (msu)
        control(!(msu_status() & 0x10), false);
}

void msu_toggle_repeat(void)
{
    repeat = !repeat;
    if (msu)
        control((msu_status() & 0x10) != 0, false);
}

bool msu_repeat(void)
{
    return repeat;
}

void msu_stop_bookmark(void)
{
    if (msu)
        control(false, true);
}

int msu_volume(void)
{
    return volume;
}

void msu_set_volume(int value)
{
    volume = value < 0 ? 0 : value > 255 ? 255 : value;
    fade_volume = -1;
    if (msu)
        reg_write(0x2006, (u8)volume);
}

void msu_fade(void)
{
    if (msu && (msu_status() & 0x10))
        fade_volume = volume;
}

// Games fade with one volume write per frame.
void msu_update(void)
{
    if (!msu || fade_volume < 0)
        return;
    fade_volume -= 4;
    if (fade_volume > 0) {
        reg_write(0x2006, (u8)fade_volume);
        return;
    }
    fade_volume = -1;
    control(false, false);
    reg_write(0x2006, (u8)volume);
}

void msu_reset(void)
{
    if (!msu)
        return;
    fade_volume = -1;
    repeat = false;
    opendspMsu1Reset(msu);
    reg_write(0x2006, (u8)volume);
}

void msu_apply(void)
{
    if (!msu)
        return;
    float mix[12];
    audio_build_mix(CH_WAV, 1, 1, mix);
    opendspMsu1SetMix(msu, mix);
    ndspChnSetInterp(CH_WAV, S.interp);
    opendspAudioStream *stream = opendspMsu1GetStream(msu);
    opendspAudioInfo info = {.struct_size = sizeof(info)};
    if (stream && R_SUCCEEDED(opendspAudioGetInfo(stream, &info)))
        opendspAudioSetRate(stream, info.sample_rate * (S.speed / 100.f));
}

u8 msu_status(void)
{
    return msu ? opendspMsu1Read(msu, 0x2000) : 0;
}

void msu_id(char out[7])
{
    for (int i = 0; i < 6; ++i)
        out[i] = msu ? (char)opendspMsu1Read(msu, 0x2002 + i) : '-';
    out[6] = 0;
}

bool msu_playing(void)
{
    return (msu_status() & 0x10) != 0;
}

opendspAudioStream *msu_stream(void)
{
    return msu ? opendspMsu1GetStream(msu) : NULL;
}

bool msu_times(float *position, float *length, float *loop)
{
    opendspAudioStream *stream = msu ? opendspMsu1GetStream(msu) : NULL;
    opendspAudioInfo info = {.struct_size = sizeof(info)};
    opendspAudioLoopInfo range = {.struct_size = sizeof(range)};
    if (!stream || R_FAILED(opendspAudioGetInfo(stream, &info)) ||
        R_FAILED(opendspAudioGetLoopInfo(stream, &range)))
        return false;
    float rate = (float)info.sample_rate;
    *position = opendspAudioGetPosition(stream) / rate;
    *length = info.total_frames / rate;
    *loop = range.available ? range.start_frame / rate : -1;
    return true;
}

const char *msu_codec(void)
{
    opendspAudioStream *stream = msu_stream();
    opendspAudioInfo info = {.struct_size = sizeof(info)};
    if (!stream || R_FAILED(opendspAudioGetInfo(stream, &info)))
        return "";
    bool dsp = opendspAudioGetBackend(stream) == OPENDSP_AAC_BACKEND_DSP;
    switch (info.format) {
    case OPENDSP_AUDIO_OPUZ:
        return dsp ? "OPUZ, DSP decode" : "OPUZ, ARM11 decode";
    case OPENDSP_AUDIO_OPUS:
        return dsp ? "Ogg Opus, DSP decode" : "Ogg Opus, ARM11 decode";
    case OPENDSP_AUDIO_VORBIS:
        return dsp ? "Ogg Vorbis, DSP decode" : "Ogg Vorbis, ARM11 decode";
    default:
        return "PCM";
    }
}
