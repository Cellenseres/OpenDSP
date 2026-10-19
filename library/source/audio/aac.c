// SPDX-License-Identifier: BSD-2-Clause
#include <stdlib.h>
#include <string.h>
#include "codec.h"

typedef struct {
    opendspAacDecoder *decoder;
    u8 input[8192];
    s16 pcm[4096 * 2];
    u32 length, offset, frames, pcm_offset;
    bool eof;
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

static Result aac_frame(audio_codec *c, bool probe)
{
    aac_data *a = c->data;
    // Each call has a fixed resynchronisation budget, even for a file of junk.
    for (unsigned attempt = 0; attempt < 32; ++attempt) {
        if (opendsp_audio_input_cancelled(&c->in))
            return OPENDSP_AUDIO_ERR_BUSY;
        if (a->offset) {
            memmove(a->input, a->input + a->offset, a->length - a->offset);
            a->length -= a->offset;
            a->offset = 0;
        }
        if (!a->eof && a->length < sizeof(a->input)) {
            size_t got;
            Result r =
                opendsp_audio_input_read(&c->in, a->input + a->length, sizeof(a->input) - a->length, &got);
            if (R_FAILED(r))
                return r;
            a->length += got;
            a->eof = feof(c->in.file);
        }
        if (!a->length && a->eof) {
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
        Result r = opendspAacDecode(a->decoder, a->input, a->length, &used, a->pcm, 4096, &frames, &info);
        c->stats.decode_ticks += svcGetSystemTick() - start;
        a->offset = used;
        if (r == OPENDSP_AAC_ERR_BUSY)
            return OPENDSP_AUDIO_ERR_BUSY;
        if (r == OPENDSP_AAC_ERR_NEED_MORE_DATA)
            return a->eof ? OPENDSP_AUDIO_ERR_TRUNCATED : OPENDSP_AUDIO_ERR_FORMAT;
        if (r == OPENDSP_AAC_INFO_SKIPPED) {
            if (!used)
                return OPENDSP_AUDIO_ERR_FORMAT;
            c->stats.skipped_bytes += used;
            continue;
        }
        if (R_FAILED(r))
            return r;
        if (!frames || !used || frames > 4096)
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
        Result r = aac_frame(c, false);
        if (R_FAILED(r))
            return r;
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
static const audio_codec_ops aac_ops = {aac_read, aac_seek, aac_close};

Result opendsp_audio_aac_open(audio_codec *c, u32 backend)
{
    aac_data *a = calloc(1, sizeof(*a));
    if (!a)
        return OPENDSP_AUDIO_ERR_MEMORY;
    c->data = a;
    c->ops = &aac_ops;
    c->info.format = OPENDSP_AUDIO_AAC;
    c->info.bits = 16;
    opendspAacConfig config = {sizeof(config), backend, 0};
    Result r = opendspAacOpen(&a->decoder, &config);
    if (R_FAILED(r))
        return r;
    r = aac_frame(c, true);
    if (r == OPENDSP_AAC_ERR_UNSUPPORTED && backend == OPENDSP_AAC_BACKEND_AUTO &&
        opendspAacGetBackend(a->decoder) == OPENDSP_AAC_BACKEND_DSP) {
        opendspAacClose(a->decoder);
        a->decoder = NULL;
        config.backend = OPENDSP_AAC_BACKEND_ARM11;
        r = opendspAacOpen(&a->decoder, &config);
        if (R_FAILED(r))
            return r;
        r = aac_seek(c, 0);
        if (R_FAILED(r))
            return r;
        r = aac_frame(c, true);
    }
    if (R_FAILED(r))
        return r;
    if (!a->frames || c->info.channels < 1 || c->info.channels > 2)
        return OPENDSP_AUDIO_ERR_FORMAT;
    c->backend = opendspAacGetBackend(a->decoder);
    c->memory = sizeof(*a) + (c->backend == OPENDSP_AAC_BACKEND_DSP ? 24832 : 32768);
    return aac_seek(c, 0);
}
