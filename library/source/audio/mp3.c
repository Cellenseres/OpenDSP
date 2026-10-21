/* SPDX-License-Identifier: BSD-2-Clause */
/* ARM11 supplies compressed bytes; DSP writes the PCM buffers. */
#include <stdlib.h>
#include <string.h>
#include "output.h"
#include "../aac_backend.h"
#include "../codec_events.h"
#include "mp3_protocol.h"

typedef struct {
    aac_dsp_backend *decoder;
    audio_output *output;
    u64 file_position, progress_at, skipped_base;
    u16 play_base, play_ack, returned;
    u32 watchdog_frames;
    bool input_eof, fresh;
} mp3_data;

static Result stream_error(u16 status)
{
    switch (status) {
    case 2:
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    case 3:
        return OPENDSP_MP3_ERR_CORRUPT;
    case 4:
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    case 6:
        return OPENDSP_MP3_ERR_CRC;
    case AAC_RESOURCE_BUSY:
        return OPENDSP_MP3_ERR_BUSY;
    default:
        return OPENDSP_MP3_ERR_BACKEND;
    }
}
static Result refill(audio_codec *c, const mp3_stream_progress *p, bool *changed)
{
    mp3_data *m = c->data;
    if (m->input_eof)
        return 0;
    size_t available = 0;
    u8 *destination = mp3_backend_stream_input(m->decoder, p->read, &available);
    if (!available)
        return 0;
    if (available < MP3_STREAM_REFILL && !(p->flags & MP3_STREAM_NEED_INPUT) && m->file_position)
        return 0;
    if (available > MP3_STREAM_REFILL)
        available = MP3_STREAM_REFILL;
    size_t got = 0;
    Result r = opendsp_audio_input_read(&c->in, destination, available, &got);
    if (R_FAILED(r))
        return r;
    if (opendsp_audio_input_cancelled(&c->in))
        return OPENDSP_AUDIO_ERR_BUSY;
    m->file_position += got;
    m->progress_at = svcGetSystemTick();
    m->input_eof = m->file_position == c->in.size;
    if (!got && !m->input_eof)
        return OPENDSP_AUDIO_ERR_IO;
    mp3_backend_stream_written(m->decoder, got, m->input_eof);
    *changed = true;
    return 0;
}
static Result mp3_seek(audio_codec *c, u64 frame)
{
    mp3_data *m = c->data;
    if (frame)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (m->fresh) {
        m->fresh = false;
        m->progress_at = svcGetSystemTick();
        return 0;
    }
    Result r = opendsp_audio_input_seek(&c->in, 0);
    if (R_FAILED(r))
        return r;
    bool preserve = m->output && !m->output->retiring;
    r = mp3_backend_stream_reset(m->decoder, preserve);
    if (R_FAILED(r))
        return r;
    m->file_position = 0;
    m->input_eof = false;
    c->position = 0;
    m->returned = 0;
    m->skipped_base = c->stats.skipped_bytes;
    if (!preserve) {
        m->play_base = m->output ? m->output->played : 0;
        m->play_ack = 0;
    }
    mp3_backend_stream_played(m->decoder, m->play_ack);
    m->progress_at = svcGetSystemTick();
    mp3_stream_progress p = {0};
    bool changed = false;
    r = refill(c, &p, &changed);
    if (changed)
        mp3_backend_stream_commit(m->decoder);
    return r;
}
static void mp3_close(audio_codec *c)
{
    mp3_data *m = c->data;
    if (m) {
        aac_backend_close(m->decoder);
        free(m);
        c->data = NULL;
        c->direct_pcm = NULL;
    }
}
static Result mp3_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    mp3_data *m = c->data;
    opendspAacStats common = {.struct_size = sizeof(common)};
    Result r = aac_backend_stats(m->decoder, &common);
    if (R_FAILED(r))
        return r;
    memcpy(stats, &common, sizeof(common));
    stats->struct_size = sizeof(*stats);
    return mp3_backend_stages(m->decoder, stats->stage_ticks);
}
static const audio_codec_ops ops = {NULL, mp3_seek, mp3_close, mp3_stats};
void opendsp_audio_mp3_stop(audio_codec *c)
{
    if (c->data)
        mp3_backend_stream_stop(((mp3_data *)c->data)->decoder);
}
void opendsp_audio_mp3_touch(audio_codec *c)
{
    if (c->data)
        ((mp3_data *)c->data)->progress_at = svcGetSystemTick();
}

Result opendsp_audio_mp3_pump(audio_codec *c, audio_output *out, bool *progress, bool *eof)
{
    mp3_data *m = c->data;
    mp3_stream_progress p;
    bool changed = false;
    m->output = out;
    *progress = false;
    *eof = false;
    u64 start = svcGetSystemTick();
    Result r = mp3_backend_stream_poll(m->decoder, &p);
    if (r == OPENDSP_MP3_ERR_BUSY)
        return OPENDSP_AUDIO_ERR_BUSY;
    if (R_FAILED(r))
        return r;
    c->stats.skipped_bytes = m->skipped_base + p.skipped;
    if (p.flags & MP3_STREAM_ERROR)
        return stream_error(p.error);
    for (unsigned n = 0; n < out->count; ++n) {
        unsigned i;
        for (i = 0; i < out->count; ++i)
            if (out->buffers[i].pending && out->buffers[i].ticket == (u16)(m->returned + 1))
                break;
        if (i == out->count)
            break;
        audio_buffer *b = &out->buffers[i];
        u16 frames = 0;
        r = mp3_backend_stream_result(m->decoder, b->ticket, &frames);
        if (r == OPENDSP_MP3_ERR_BUSY)
            break;
        if (R_FAILED(r))
            return r;
        b->pending = false;
        ++m->returned;
        m->progress_at = svcGetSystemTick();
        if (frames) {
            c->position += frames;
            c->stats.decoded_frames += frames;
            opendsp_audio_output_queue(out, i, frames);
        }
        *progress = true;
    }
    u16 played = out->played - m->play_base;
    if (played != m->play_ack) {
        mp3_backend_stream_played(m->decoder, played);
        m->play_ack = played;
        changed = true;
        m->progress_at = svcGetSystemTick();
    }
    if (p.flags & MP3_STREAM_DRAINED) {
        for (unsigned i = 0; i < out->count; ++i)
            out->buffers[i].pending = false;
        c->info.total_frames = c->position;
        c->info.duration_known = true;
        *eof = true;
    } else {
        r = refill(c, &p, &changed);
        if (R_FAILED(r))
            return r;
        int slot;
        while ((slot = opendsp_audio_output_free(out)) >= 0) {
            audio_buffer *b = &out->buffers[slot];
            r = mp3_backend_stream_submit(m->decoder, slot, &b->ticket);
            if (r == OPENDSP_MP3_ERR_BUSY)
                break;
            if (R_FAILED(r))
                return r;
            b->pending = true;
            changed = true;
            *progress = true;
        }
    }
    c->stats.decode_ticks += svcGetSystemTick() - start;
    if (changed)
        mp3_backend_stream_commit(m->decoder);
    if (svcGetSystemTick() - m->progress_at > 2ull * SYSCLOCK_ARM11) {
        /* Large output buffers and a slow emulator may span many MP3 frames
         * without a buffer event. Check actual decoder progress before calling
         * this a stall; healthy playback never needs this mailbox query. */
        opendspAacStats live = {.struct_size = sizeof(live)};
        r = aac_backend_stats(m->decoder, &live);
        if (R_FAILED(r) || live.frames == m->watchdog_frames)
            return OPENDSP_MP3_ERR_TIMEOUT;
        m->watchdog_frames = live.frames;
        m->progress_at = svcGetSystemTick();
    }
    return 0;
}
Result opendsp_audio_mp3_open(audio_codec *c, u32 backend, u32 count, u32 frames, u32 flags)
{
    if (backend == OPENDSP_MP3_BACKEND_ARM11)
        return opendsp_audio_mp3_arm11_open(c);
    if (backend != OPENDSP_MP3_BACKEND_AUTO && backend != OPENDSP_MP3_BACKEND_DSP)
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    c->ops = &ops;
    c->info.format = OPENDSP_AUDIO_MP3;
    c->info.bits = 16;
    mp3_data *m = calloc(1, sizeof(*m));
    if (!m)
        return OPENDSP_AUDIO_ERR_MEMORY;
    c->data = m;
    Result r = mp3_backend_stream_open(&m->decoder, count, frames, (flags & OPENDSP_AUDIO_PROFILE) != 0);
    if (R_FAILED(r))
        return r;
    c->backend = OPENDSP_MP3_BACKEND_DSP;
    c->direct_pcm = mp3_backend_stream_pcm(m->decoder, &c->direct_stride);
    c->memory = sizeof(*m) + mp3_backend_stream_overhead(); /* borrowed PCM counted by output.bytes */
    u64 deadline = svcGetSystemTick() + (u64)SYSCLOCK_ARM11;
    for (;;) {
        mp3_stream_progress p;
        bool changed = false;
        r = mp3_backend_stream_poll(m->decoder, &p);
        if (r == OPENDSP_MP3_ERR_BUSY) {
            u64 now = svcGetSystemTick();
            if (now >= deadline)
                return OPENDSP_MP3_ERR_TIMEOUT;
            opendsp_codec_wait((s64)((deadline - now) * 1000000000ull / SYSCLOCK_ARM11));
            continue;
        }
        if (R_FAILED(r))
            return r;
        if (p.flags & MP3_STREAM_ERROR)
            return stream_error(p.error);
        if (p.flags & MP3_STREAM_HEADER) {
            static const u32 rates[3] = {44100, 48000, 32000};
            c->info.channels = p.channels;
            c->info.sample_rate = rates[p.rate];
            c->info.aac_flags = OPENDSP_MP3_INFO_CHANGED;
            m->fresh = true;
            return 0;
        }
        r = refill(c, &p, &changed);
        if (R_FAILED(r))
            return r;
        if (changed) {
            mp3_backend_stream_commit(m->decoder);
            deadline = svcGetSystemTick() + (u64)SYSCLOCK_ARM11;
        }
        u64 now = svcGetSystemTick();
        if (now >= deadline)
            return OPENDSP_MP3_ERR_TIMEOUT;
        opendsp_codec_wait((s64)((deadline - now) * 1000000000ull / SYSCLOCK_ARM11));
    }
}
