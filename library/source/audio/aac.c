// SPDX-License-Identifier: BSD-2-Clause
#include <stdlib.h>
#include <string.h>
#include "codec.h"

#define AAC_INPUT_BYTES 8192
#define AAC_PCM_FRAMES 4096
#define AAC_BLOCK_FRAMES 1024
#define AAC_SYNC_ATTEMPTS 32

typedef struct {
    opendspAacDecoder *decoder;
    u8 input[AAC_INPUT_BYTES];
    u32 length, offset, frames, pcm_offset, pcm_capacity;
    bool eof;
    s16 pcm[];
} aac_data;

static Result aac_seek(audio_codec *c, u64 frame)
{
    if (frame)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    Result r = opendsp_audio_input_seek(&c->in, 0);
    if (R_FAILED(r))
        return r;
    aac_data *a = c->data;
    opendspAacReset(a->decoder);
    a->length = a->offset = a->frames = a->pcm_offset = 0;
    a->eof = false;
    c->position = 0;
    return 0;
}

static Result aac_refill(audio_codec *c)
{
    aac_data *a = c->data;
    u32 left = a->length - a->offset;
    if (left == sizeof(a->input))
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (left && a->offset)
        memmove(a->input, a->input + a->offset, left);
    a->length = left;
    a->offset = 0;
    size_t got;
    Result r = opendsp_audio_input_read(&c->in, a->input + left, sizeof(a->input) - left, &got);
    if (R_FAILED(r))
        return r;
    a->length += got;
    a->eof = feof(c->in.file);
    return !got && !a->eof ? OPENDSP_AUDIO_ERR_IO : 0;
}

static Result aac_frame(audio_codec *c, bool probe, s16 *pcm, u32 capacity)
{
    aac_data *a = c->data;
    for (unsigned attempt = 0; attempt < AAC_SYNC_ATTEMPTS; ++attempt) {
        if (opendsp_audio_input_cancelled(&c->in))
            return OPENDSP_AUDIO_ERR_BUSY;
        if (a->offset == a->length && !a->eof) {
            Result r = aac_refill(c);
            if (R_FAILED(r))
                return r;
        }
        if (a->offset == a->length && a->eof) {
            if (!probe) {
                c->info.total_frames = c->position;
                c->info.duration_known = true;
            }
            a->frames = a->pcm_offset = 0;
            return 0;
        }
        size_t used = 0, frames = 0;
        opendspAacInfo info = {0};
        u64 start = svcGetSystemTick();
        Result r = opendspAacDecode(a->decoder, a->input + a->offset, a->length - a->offset, &used, pcm,
                                    capacity, &frames, &info);
        c->stats.decode_ticks += svcGetSystemTick() - start;
        a->offset += used;
        if (r == OPENDSP_AAC_ERR_BUSY)
            return OPENDSP_AUDIO_ERR_BUSY;
        if (r == OPENDSP_AAC_ERR_NEED_MORE_DATA) {
            if (a->eof)
                return OPENDSP_AUDIO_ERR_TRUNCATED;
            r = aac_refill(c);
            if (R_FAILED(r))
                return r;
            continue;
        }
        if (r == OPENDSP_AAC_INFO_SKIPPED) {
            if (!used)
                return OPENDSP_AUDIO_ERR_FORMAT;
            c->stats.skipped_bytes += used;
            continue;
        }
        if (R_FAILED(r))
            return r;
        if (!frames || !used || frames > capacity)
            return OPENDSP_AUDIO_ERR_FORMAT;
        if (info.flags & OPENDSP_AAC_INFO_DISCONTINUITY)
            return OPENDSP_AUDIO_ERR_CONTEXT;
        if (!probe && (info.sample_rate != c->info.sample_rate || info.channels != c->info.channels))
            return OPENDSP_AUDIO_ERR_UNSUPPORTED;
        c->info.sample_rate = info.sample_rate;
        c->info.channels = info.channels;
        c->info.aac_flags |= info.flags;
        a->frames = frames;
        a->pcm_offset = 0;
        c->stats.decoded_frames += frames;
        return 0;
    }
    return OPENDSP_AUDIO_ERR_FORMAT;
}

static Result aac_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    aac_data *a = c->data;
    *frames = 0;
    if (a->pcm_offset == a->frames) {
        // A changed ADTS header can write stereo before the format check.
        bool direct = c->info.channels == 2 && capacity >= AAC_BLOCK_FRAMES && capacity >= a->frames;
        Result r = aac_frame(c, false, direct ? out : a->pcm, direct ? capacity : a->pcm_capacity);
        if (direct && r == OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL) {
            direct = false;
            r = aac_frame(c, false, a->pcm, a->pcm_capacity);
        }
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
    memcpy(out, a->pcm + a->pcm_offset * c->info.channels, count * c->info.channels * 2);
    a->pcm_offset += count;
    c->position += count;
    *frames = count;
    return 0;
}

static void aac_close(audio_codec *c)
{
    aac_data *a = c->data;
    if (a) {
        opendspAacClose(a->decoder);
        free(a);
        c->data = NULL;
    }
}
static Result aac_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    aac_data *a = c->data;
    opendspAacStats common = {.struct_size = sizeof(common)};
    Result r = opendspAacGetStats(a->decoder, &common);
    if (R_FAILED(r))
        return r;
    memset(stats, 0, sizeof(*stats));
    memcpy(stats, &common, sizeof(common));
    stats->struct_size = sizeof(*stats);
    return 0;
}
static const audio_codec_ops aac_ops = {aac_read, aac_seek, aac_close, aac_stats};

static Result aac_allocate(audio_codec *c, u32 backend)
{
    opendspAacConfig config = {sizeof(config), backend, 0};
    opendspAacDecoder *decoder = NULL;
    Result r = opendspAacOpen(&decoder, &config);
    if (R_FAILED(r))
        return r;
    backend = opendspAacGetBackend(decoder);
    u32 capacity = backend == OPENDSP_AAC_BACKEND_DSP ? AAC_BLOCK_FRAMES : AAC_PCM_FRAMES;
    size_t bytes = sizeof(aac_data) + capacity * 2 * sizeof(s16);
    aac_data *a = calloc(1, bytes);
    if (!a) {
        opendspAacClose(decoder);
        return OPENDSP_AUDIO_ERR_MEMORY;
    }
    a->decoder = decoder;
    a->pcm_capacity = capacity;
    c->data = a;
    c->backend = backend;
    c->memory = bytes + (backend == OPENDSP_AAC_BACKEND_DSP ? 24832 : 32768);
    return 0;
}

Result opendsp_audio_aac_open(audio_codec *c, u32 backend)
{
    c->ops = &aac_ops;
    c->info.format = OPENDSP_AUDIO_AAC;
    c->info.bits = 16;
    Result r = aac_allocate(c, backend);
    if (R_FAILED(r))
        return r;
    aac_data *a = c->data;
    r = aac_frame(c, true, a->pcm, a->pcm_capacity);
    if (r == OPENDSP_AAC_ERR_UNSUPPORTED && backend == OPENDSP_AAC_BACKEND_AUTO &&
        c->backend == OPENDSP_AAC_BACKEND_DSP) {
        aac_close(c);
        r = aac_allocate(c, OPENDSP_AAC_BACKEND_ARM11);
        if (R_FAILED(r))
            return r;
        a = c->data;
        r = aac_seek(c, 0);
        if (R_FAILED(r))
            return r;
        r = aac_frame(c, true, a->pcm, a->pcm_capacity);
    }
    if (R_FAILED(r))
        return r;
    if (!a->frames || c->info.channels < 1 || c->info.channels > 2)
        return OPENDSP_AUDIO_ERR_FORMAT;
    return aac_seek(c, 0);
}
