// SPDX-License-Identifier: BSD-2-Clause
#include <stdlib.h>
#include <string.h>
#include "codec.h"

typedef struct {
    u64 offset, bytes;
    u32 frame_bytes;
} wav_data;
static u16 rd16(const u8 *p)
{
    return p[0] | (u16)p[1] << 8;
}
static u32 rd32(const u8 *p)
{
    return rd16(p) | (u32)rd16(p + 2) << 16;
}

static Result exact(audio_codec *c, void *p, size_t n)
{
    size_t got;
    Result r = opendsp_audio_input_read(&c->in, p, n, &got);
    return R_FAILED(r) ? r : got == n ? 0 : OPENDSP_AUDIO_ERR_TRUNCATED;
}

static Result wav_seek(audio_codec *c, u64 frame)
{
    wav_data *w = c->data;
    if (frame > c->info.total_frames)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    Result r = opendsp_audio_input_seek(&c->in, w->offset + frame * w->frame_bytes);
    if (R_SUCCEEDED(r))
        c->position = frame;
    return r;
}

static Result wav_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    wav_data *w = c->data;
    u64 left = c->info.total_frames - c->position;
    u32 count = left < capacity ? (u32)left : capacity;
    *frames = 0;
    if (!count)
        return 0;
    Result r = exact(c, out, count * w->frame_bytes);
    if (R_FAILED(r))
        return r;
    if (c->info.bits == 8)
        for (u32 i = 0; i < count * w->frame_bytes; ++i)
            ((u8 *)out)[i] ^= 0x80;
    c->position += count;
    *frames = count;
    c->stats.decoded_frames += count;
    return 0;
}

static void wav_close(audio_codec *c)
{
    free(c->data);
    c->data = NULL;
}
static const audio_codec_ops wav_ops = {wav_read, wav_seek, wav_close};

Result opendsp_audio_wav_open(audio_codec *c)
{
    u8 h[16];
    Result r = exact(c, h, 12);
    if (R_FAILED(r))
        return r;
    if (memcmp(h, "RIFF", 4) || memcmp(h + 8, "WAVE", 4))
        return OPENDSP_AUDIO_ERR_FORMAT;
    u64 end = 8ull + rd32(h + 4), pos = 12;
    if (end < 12 || end > c->in.size)
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    bool fmt = false, data = false;
    wav_data w = {0};
    while (pos < end) {
        if (end - pos < 8)
            return OPENDSP_AUDIO_ERR_FORMAT;
        r = opendsp_audio_input_seek(&c->in, pos);
        if (R_FAILED(r))
            return r;
        r = exact(c, h, 8);
        if (R_FAILED(r))
            return r;
        u32 size = rd32(h + 4);
        u64 next = pos + 8 + (u64)size + (size & 1);
        if (next > end)
            return OPENDSP_AUDIO_ERR_TRUNCATED;
        if (!memcmp(h, "fmt ", 4)) {
            if (fmt || size < 16)
                return OPENDSP_AUDIO_ERR_FORMAT;
            r = exact(c, h, 16);
            if (R_FAILED(r))
                return r;
            if (rd16(h) != 1)
                return OPENDSP_AUDIO_ERR_UNSUPPORTED;
            c->info.channels = rd16(h + 2);
            c->info.sample_rate = rd32(h + 4);
            c->info.bits = rd16(h + 14);
            if ((c->info.channels != 1 && c->info.channels != 2) ||
                (c->info.bits != 8 && c->info.bits != 16) || c->info.sample_rate < 1000 ||
                c->info.sample_rate > 4 * 32728)
                return OPENDSP_AUDIO_ERR_UNSUPPORTED;
            w.frame_bytes = c->info.channels * c->info.bits / 8;
            if (rd16(h + 12) != w.frame_bytes || rd32(h + 8) != c->info.sample_rate * w.frame_bytes)
                return OPENDSP_AUDIO_ERR_FORMAT;
            fmt = true;
        } else if (!memcmp(h, "data", 4)) {
            if (data)
                return OPENDSP_AUDIO_ERR_FORMAT;
            w.offset = pos + 8;
            w.bytes = size;
            data = true;
        }
        pos = next;
    }
    if (!fmt || !data || w.bytes % w.frame_bytes)
        return OPENDSP_AUDIO_ERR_FORMAT;
    c->data = malloc(sizeof(w));
    if (!c->data)
        return OPENDSP_AUDIO_ERR_MEMORY;
    memcpy(c->data, &w, sizeof(w));
    c->memory = sizeof(w);
    c->ops = &wav_ops;
    c->info.format = OPENDSP_AUDIO_WAV;
    c->info.duration_known = true;
    c->info.total_frames = w.bytes / w.frame_bytes;
    return wav_seek(c, 0);
}
