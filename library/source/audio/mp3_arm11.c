/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdlib.h>
#include <string.h>
#include "codec.h"
#include "../mp3_arm11.h"

typedef struct {
    opendspMp3Decoder *decoder;
    u64 start, end;
    u32 length, offset, frames, pcm_offset, resync;
    u8 input[4096];
    s16 pcm[2304];
} arm_stream;

static Result arm_seek(audio_codec *c, u64 frame)
{
    if (frame)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    arm_stream *a = c->data;
    Result r = opendsp_audio_input_seek(&c->in, a->start);
    if (R_FAILED(r))
        return r;
    opendspMp3Reset(a->decoder);
    a->length = a->offset = a->frames = a->pcm_offset = a->resync = 0;
    c->position = 0;
    return 0;
}
static Result arm_refill(audio_codec *c)
{
    arm_stream *a = c->data;
    u32 left = a->length - a->offset;
    if (left == sizeof(a->input))
        return OPENDSP_AUDIO_ERR_FORMAT;
    memmove(a->input, a->input + a->offset, left);
    a->length = left;
    a->offset = 0;
    long at = ftell(c->in.file);
    if (at < 0 || (u64)at > a->end)
        return OPENDSP_AUDIO_ERR_IO;
    size_t count = sizeof(a->input) - left, got = 0;
    if (count > a->end - (u64)at)
        count = (size_t)(a->end - (u64)at);
    Result r = opendsp_audio_input_read(&c->in, a->input + left, count, &got);
    a->length += got;
    if (R_FAILED(r))
        return r;
    return got < count ? OPENDSP_AUDIO_ERR_TRUNCATED : 0;
}
static Result arm_frame(audio_codec *c, bool probe, s16 *out)
{
    arm_stream *a = c->data;
    for (;;) {
        if (opendsp_audio_input_cancelled(&c->in))
            return OPENDSP_AUDIO_ERR_BUSY;
        long at = ftell(c->in.file);
        if (at < 0)
            return OPENDSP_AUDIO_ERR_IO;
        bool eof = (u64)at == a->end;
        if (a->offset == a->length && eof) {
            a->frames = a->pcm_offset = 0;
            if (!probe) {
                c->info.total_frames = c->position;
                c->info.duration_known = true;
            }
            return 0;
        }
        size_t used = 0, frames = 0;
        opendspMp3Info info = {0};
        u64 begin = svcGetSystemTick();
        Result r = opendspMp3Decode(a->decoder, a->input + a->offset, a->length - a->offset, &used, out, 1152,
                                    &frames, &info);
        c->stats.decode_ticks += svcGetSystemTick() - begin;
        a->offset += used;
        if (r == OPENDSP_MP3_ERR_NEED_MORE_DATA) {
            if (eof)
                return OPENDSP_AUDIO_ERR_TRUNCATED;
            r = arm_refill(c);
            if (R_FAILED(r))
                return r;
            continue;
        }
        if (r == OPENDSP_MP3_INFO_SKIPPED) {
            if (!used)
                return OPENDSP_AUDIO_ERR_FORMAT;
            c->stats.skipped_bytes += used;
            /* Reservoir priming consumes a valid frame and is not resync junk. */
            if (info.frame_samples)
                a->resync = 0;
            else if ((a->resync += used) > 4096)
                return OPENDSP_MP3_ERR_CORRUPT;
            continue;
        }
        if (R_FAILED(r))
            return r;
        if (!used || frames != 1152)
            return OPENDSP_AUDIO_ERR_FORMAT;
        if (!probe && (info.sample_rate != c->info.sample_rate || info.channels != c->info.channels))
            return OPENDSP_AUDIO_ERR_UNSUPPORTED;
        c->info.sample_rate = info.sample_rate;
        c->info.channels = info.channels;
        c->info.aac_flags |= info.flags;
        a->frames = frames;
        a->pcm_offset = a->resync = 0;
        c->stats.decoded_frames += frames;
        return 0;
    }
}
static Result arm_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    arm_stream *a = c->data;
    *frames = 0;
    if (a->pcm_offset == a->frames) {
        /* Mono can change to stereo in a rejected header: use the stereo scratch. */
        bool direct = c->info.channels == 2 && capacity >= 1152;
        Result r = arm_frame(c, false, direct ? out : a->pcm);
        if (R_FAILED(r))
            return r;
        if (direct) {
            a->pcm_offset = a->frames;
            c->position += a->frames;
            *frames = a->frames;
            return 0;
        }
    }
    u32 count = a->frames - a->pcm_offset;
    if (count > capacity)
        count = capacity;
    memcpy(out, a->pcm + a->pcm_offset * c->info.channels, count * c->info.channels * sizeof(s16));
    a->pcm_offset += count;
    c->position += count;
    *frames = count;
    return 0;
}
static void arm_close(audio_codec *c)
{
    arm_stream *a = c->data;
    if (a) {
        opendspMp3Close(a->decoder);
        free(a);
        c->data = NULL;
    }
}
static Result arm_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    (void)c;
    (void)stats;
    return OPENDSP_MP3_ERR_BACKEND;
}
static const audio_codec_ops arm_ops = {arm_read, arm_seek, arm_close, arm_stats};

Result opendsp_audio_mp3_arm11_open(audio_codec *c)
{
    c->ops = &arm_ops;
    c->backend = OPENDSP_MP3_BACKEND_ARM11;
    c->info.format = OPENDSP_AUDIO_MP3;
    c->info.bits = 16;
    arm_stream *a = calloc(1, sizeof(*a));
    if (!a)
        return OPENDSP_AUDIO_ERR_MEMORY;
    c->data = a;
    a->end = c->in.size;
    opendspMp3Config config = {sizeof(config), OPENDSP_MP3_BACKEND_ARM11, 0};
    Result r = opendspMp3Open(&a->decoder, &config);
    if (R_FAILED(r))
        return r;
    c->memory = sizeof(*a) + mp3_arm11_bytes() + 2 * sizeof(void *);
    u8 tag[10];
    size_t got = 0;
    r = opendsp_audio_input_read(&c->in, tag, sizeof(tag), &got);
    if (R_FAILED(r))
        return r;
    if (got >= 3 && !memcmp(tag, "ID3", 3)) {
        if (got < 10)
            return OPENDSP_AUDIO_ERR_TRUNCATED;
        if (tag[3] < 2 || tag[3] > 4 || tag[4] == 255)
            return OPENDSP_MP3_ERR_CORRUPT;
        u32 size = 0;
        for (unsigned i = 6; i < 10; ++i) {
            if (tag[i] & 128)
                return OPENDSP_MP3_ERR_CORRUPT;
            size = (size << 7) | tag[i];
        }
        if (size > 16777216u)
            return OPENDSP_MP3_ERR_UNSUPPORTED;
        a->start = (u64)size + 10 + ((tag[3] == 4 && (tag[5] & 16)) ? 10 : 0);
        if (a->start > a->end)
            return OPENDSP_AUDIO_ERR_TRUNCATED;
        c->stats.skipped_bytes += a->start;
    }
    if (a->end - a->start >= 128) {
        r = opendsp_audio_input_seek(&c->in, a->end - 128);
        if (R_FAILED(r))
            return r;
        r = opendsp_audio_input_read(&c->in, tag, 3, &got);
        if (R_FAILED(r))
            return r;
        if (got == 3 && !memcmp(tag, "TAG", 3)) {
            a->end -= 128;
            c->stats.skipped_bytes += 128;
        }
    }
    r = arm_seek(c, 0);
    if (R_FAILED(r))
        return r;
    r = arm_frame(c, true, a->pcm);
    if (R_FAILED(r))
        return r;
    if (!a->frames)
        return OPENDSP_AUDIO_ERR_FORMAT;
    return arm_seek(c, 0);
}
