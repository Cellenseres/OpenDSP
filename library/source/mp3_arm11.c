/* SPDX-License-Identifier: BSD-2-Clause */
/* Shared integer decoder, compiled for ARM11. */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <3ds/synchronization.h>
#include "mp3_arm11.h"

static void arm_emit(void *context, const u16 *pcm, u16 frames);
#define MP3_EMIT(d, p, n) arm_emit(d, p, n)
#define MP3_YIELD(d) ((void)0)
#define mp3_reset opendsp_mp3_core_reset
#define mp3_frame opendsp_mp3_core_frame
#define mp3_product opendsp_mp3_core_product
#include "../../firmware/mp3/decoder.c"

static LightLock decoder_lock;

__attribute__((constructor)) static void init_decoder_lock(void)
{
    LightLock_Init(&decoder_lock);
}

struct mp3_arm11 {
    mp3_decoder core; /* first member: the emit callback receives this pointer */
    mp3_state state;
    s32 spectrum[1152], work[2112], overlap[2][288];
    u16 input[721], main_data[1024];
    s16 *pcm;
    size_t emitted;
    bool changed;
};

s32 mp3_product(s32 a, s32 b, s16 shift)
{
    int64_t v = ((int64_t)a * b + ((int64_t)1 << (shift - 1))) >> shift;
    if (v > INT32_MAX)
        return INT32_MAX;
    if (v < INT32_MIN)
        return INT32_MIN;
    return (s32)v;
}
static void arm_emit(void *context, const u16 *pcm, u16 frames)
{
    mp3_arm11 *d = context;
    memcpy(d->pcm + d->emitted * d->state.channels, pcm, frames * d->state.channels * sizeof(s16));
    d->emitted += frames;
}
mp3_arm11 *mp3_arm11_open(void)
{
    mp3_arm11 *d = calloc(1, sizeof(*d));
    if (!d)
        return NULL;
    d->core.s = &d->state;
    d->core.spectrum = d->spectrum;
    d->core.work = d->work;
    d->core.overlap[0] = (u16 *)d->overlap[0];
    d->core.overlap[1] = (u16 *)d->overlap[1];
    d->core.input = d->input;
    d->core.main_data = d->main_data;
    mp3_arm11_reset(d);
    return d;
}
void mp3_arm11_reset(mp3_arm11 *d)
{
    if (d) {
        mp3_reset(&d->core);
        d->changed = true;
        d->pcm = NULL;
        d->emitted = 0;
    }
}
void mp3_arm11_close(mp3_arm11 *d)
{
    free(d);
}
size_t mp3_arm11_bytes(void)
{
    return sizeof(mp3_arm11);
}

Result mp3_arm11_decode(mp3_arm11 *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                        size_t capacity, size_t *frames, opendspMp3Info *info)
{
    static const u16 bitrates[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
    static const u32 rates[3] = {44100, 48000, 32000};
    const u8 *p = input;
    size_t skip = 0;
    if (used)
        *used = 0;
    if (frames)
        *frames = 0;
    if (info)
        memset(info, 0, sizeof(*info));
    if (!d || !used || !frames || !pcm || (!p && bytes))
        return OPENDSP_MP3_ERR_INVALID_ARG;
    if (!bytes)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    while (skip + 1 < bytes && (p[skip] != 255 || (p[skip + 1] & 0xe6) != 0xe2))
        ++skip;
    if (skip + 1 >= bytes)
        skip = p[bytes - 1] == 255 ? bytes - 1 : bytes;
    if (skip) {
        *used = skip;
        return OPENDSP_MP3_INFO_SKIPPED;
    }
    if (bytes < 4)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    if ((p[1] & 0x18) != 0x18 || !(p[2] >> 4)) {
        *used = 4;
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    }
    unsigned sf = p[2] >> 2 & 3, kbps = bitrates[p[2] >> 4];
    if (sf == 3 || !kbps || (p[3] & 3) == 2) {
        *used = 1;
        return OPENDSP_MP3_INFO_SKIPPED;
    }
    unsigned channels = (p[3] >> 6) == 3 ? 1 : 2;
    unsigned length = 144000u * kbps / rates[sf] + (p[2] >> 1 & 1);
    if (bytes < length)
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    if (info) {
        info->sample_rate = rates[sf];
        info->channels = channels;
        info->frame_samples = 1152;
        info->bitrate_kbps = kbps;
    }
    if ((p[3] & 0xd0) == 0x50 || (p[3] & 3)) {
        *used = length;
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    }
    if (capacity < 1152)
        return OPENDSP_MP3_ERR_OUTPUT_TOO_SMALL;
    memset(d->input, 0, (length + 1) / 2 * sizeof(u16));
    for (unsigned i = 0; i < length; ++i)
        d->input[i >> 1] |= (u16)p[i] << (8 * (i & 1));
    d->pcm = pcm;
    d->emitted = 0;
    /* Scalar helpers share the current-decoder pointer. */
    LightLock_Lock(&decoder_lock);
    u16 status = mp3_frame(&d->core, (u16)length);
    LightLock_Unlock(&decoder_lock);
    d->pcm = NULL;
    *used = length;
    switch (status) {
    case MP3_OK:
        break;
    case MP3_HISTORY:
        return OPENDSP_MP3_INFO_SKIPPED;
    case MP3_TRUNCATED:
        return OPENDSP_MP3_ERR_NEED_MORE_DATA;
    case MP3_CORRUPT:
        return OPENDSP_MP3_ERR_CORRUPT;
    case MP3_UNSUPPORTED:
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    case MP3_CRC:
        return OPENDSP_MP3_ERR_CRC;
    default:
        return OPENDSP_MP3_ERR_INVALID_ARG;
    }
    if (d->emitted != 1152)
        return OPENDSP_MP3_ERR_CORRUPT;
    *frames = d->emitted;
    if (info && d->changed)
        info->flags = OPENDSP_MP3_INFO_CHANGED;
    d->changed = false;
    return 0;
}
