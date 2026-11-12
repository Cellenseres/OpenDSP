/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdlib.h>
#include <string.h>
#include <opus.h>
#include "ogg.h"
#include "../internal.h"
#ifdef OPENDSP_CODEC_BANKS
#include "../ogg_backend.h"
#endif

// zelda3 OPUZ: play ranges, then length-prefixed Opus packets.
#define OPUZ_RANGES 64
#define OPUZ_PACKET 1275
#define OPUZ_FRAMES 5760
#define OPUZ_MARK_SPACING 48000u
// 80 ms settles CELT, as in opusfile.
#define OPUZ_PREROLL 3840u
#define OPUZ_RING 8

typedef struct {
    u64 sample, offset; // decoder timeline, packet start
} opuz_mark;

typedef struct {
    audio_ogg_io io;
    OpusDecoder *arm;
#ifdef OPENDSP_CODEC_BANKS
    aac_dsp_backend *dsp;
#endif
    bool fresh, has_repeat;
    u32 lookahead;
    u64 data;
    opuz_mark repeat;
    opuz_mark *marks;
    u32 mark_count, mark_capacity;
    u64 next_offset, next_sample;
    u32 skip, pcm_at, pcm_frames;
    u8 packet[OPUZ_PACKET + 1];
    s16 pcm[OPUZ_FRAMES * 2];
} opuz_data;

static u32 rd32(const u8 *p)
{
    return p[0] | p[1] << 8 | p[2] << 16 | (u32)p[3] << 24;
}

static u32 opuz_memory(const opuz_data *o)
{
    u32 bytes = sizeof(*o) + o->mark_capacity * sizeof(opuz_mark) + o->io.memory.peak;
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp) bytes += aac_backend_memory(o->dsp);
#endif
    return bytes;
}

// Seek index, roughly one mark per second.
static void add_mark(opuz_data *o, u64 sample, u64 offset)
{
    if (o->mark_count && sample < o->marks[o->mark_count - 1].sample + OPUZ_MARK_SPACING)
        return;
    if (o->mark_count == o->mark_capacity) {
        u32 capacity = o->mark_capacity ? o->mark_capacity * 2 : 64;
        opuz_mark *grown = realloc(o->marks, capacity * sizeof(*grown));
        if (!grown)
            return;
        o->marks = grown;
        o->mark_capacity = capacity;
    }
    o->marks[o->mark_count++] = (opuz_mark){sample, offset};
}

// Full reads fill o->packet; others read the TOC only.
static Result packet_at(audio_codec *c, opuz_data *o, u64 offset, bool full, const u8 **data, u32 *bytes,
                        u32 *frames, u64 *next)
{
    u8 h[2];
    size_t got;
    Result r = opendsp_audio_input_seek(&c->in, offset);
    if (R_SUCCEEDED(r))
        r = opendsp_audio_input_read(&c->in, h, sizeof(h), &got);
    if (R_FAILED(r))
        return r;
    if (got != sizeof(h))
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    u32 size = (h[0] | h[1] << 8) & 0x7fff;
    u32 implied = h[1] >> 7;
    if (size > OPUZ_PACKET || (!size && !implied))
        return OPENDSP_AUDIO_ERR_FORMAT;
    u32 want = full || size < 2 ? size : 2;
    o->packet[0] = 0xfc;
    r = opendsp_audio_input_read(&c->in, o->packet + 1, want, &got);
    if (R_FAILED(r))
        return r;
    if (got != want)
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    *data = o->packet + 1 - implied;
    *bytes = size + implied;
    int n = opus_packet_get_nb_samples(*data, (opus_int32)(want + implied), 48000);
    if (n <= 0 || n > OPUZ_FRAMES)
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (c->info.channels && opus_packet_get_nb_channels(*data) != c->info.channels)
        return OPENDSP_AUDIO_ERR_FORMAT;
    *frames = (u32)n;
    *next = offset + 2 + size;
    return 0;
}

static Result decode_next(audio_codec *c, opuz_data *o)
{
    const u8 *data;
    u32 bytes, frames;
    u64 next;
    Result r = packet_at(c, o, o->next_offset, true, &data, &bytes, &frames, &next);
    if (R_FAILED(r))
        return r;
    add_mark(o, o->next_sample, o->next_offset);
    u64 begin = svcGetSystemTick();
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp) {
        if (!ogg_backend_context_valid(o->dsp)) {
            if (!ogg_backend_recoverable(o->dsp))
                return OPENDSP_AUDIO_ERR_CONTEXT;
            o->fresh = true;
        }
        if (o->fresh)
            r = ogg_backend_reset(o->dsp, c->in.generation, c->in.expected);
        if (R_SUCCEEDED(r))
            r = ogg_backend_decode(o->dsp, data, bytes, o->pcm, (u16)frames, 0, c->in.generation,
                                   c->in.expected);
        if (R_FAILED(r)) {
            o->fresh = true;
            return r;
        }
    } else
#endif
    {
        o->io.error = 0;
        AUDIO_OGG_ENTER(&o->io);
        if (o->fresh)
            opus_decoder_ctl(o->arm, OPUS_RESET_STATE);
        int n = opus_decode(o->arm, data, (opus_int32)bytes, o->pcm, OPUZ_FRAMES, 0);
        opendsp_audio_ogg_leave();
        if (n != (int)frames)
            return OPENDSP_AUDIO_ERR_FORMAT;
    }
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    o->fresh = false;
    o->next_offset = next;
    o->next_sample += frames;
    o->pcm_frames = frames;
    o->pcm_at = o->skip < frames ? o->skip : frames;
    o->skip -= o->pcm_at;
    return 0;
}

static Result opuz_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    opuz_data *o = c->data;
    u16 channels = c->info.channels;
    s16 *dst = out;
    *frames = 0;
    while (*frames < capacity && c->position < c->info.total_frames) {
        if (o->pcm_at == o->pcm_frames) {
            Result r = decode_next(c, o);
            if (R_FAILED(r) && !*frames)
                return r;
            if (R_FAILED(r))
                break;
            continue;
        }
        u64 left = c->info.total_frames - c->position;
        u32 n = o->pcm_frames - o->pcm_at;
        if (n > capacity - *frames)
            n = capacity - *frames;
        if (n > left)
            n = (u32)left;
        memcpy(dst + (size_t)*frames * channels, o->pcm + (size_t)o->pcm_at * channels,
               (size_t)n * channels * sizeof(s16));
        o->pcm_at += n;
        *frames += n;
        c->position += n;
    }
    c->stats.decoded_frames += *frames;
    c->memory = opuz_memory(o);
    return 0;
}

static Result opuz_seek(audio_codec *c, u64 frame)
{
    opuz_data *o = c->data;
    if (frame > c->info.total_frames)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    o->pcm_at = o->pcm_frames = 0;
    o->fresh = true;
    if (frame == c->info.total_frames) {
        c->position = frame;
        return 0;
    }
    u64 target = frame + o->lookahead;
    u64 settle = target > OPUZ_PREROLL ? target - OPUZ_PREROLL : 0;
    opuz_mark at = {0, o->data};
    for (u32 i = 0; i < o->mark_count && o->marks[i].sample <= settle; ++i)
        at = o->marks[i];
    if (o->has_repeat && o->repeat.sample <= settle && o->repeat.sample > at.sample)
        at = o->repeat;
    // Recent packets; decoding starts at the oldest after settle.
    opuz_mark ring[OPUZ_RING];
    u32 seen = 0;
    u64 begin = svcGetSystemTick();
    for (;;) {
        const u8 *data;
        u32 bytes, frames;
        u64 next;
        Result r = packet_at(c, o, at.offset, false, &data, &bytes, &frames, &next);
        if (R_FAILED(r))
            return r;
        add_mark(o, at.sample, at.offset);
        ring[seen++ % OPUZ_RING] = at;
        if (target < at.sample + frames)
            break;
        at.sample += frames;
        at.offset = next;
    }
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    u32 pick = seen > OPUZ_RING ? seen - OPUZ_RING : 0;
    for (u32 i = pick; i < seen; ++i)
        if (ring[i % OPUZ_RING].sample <= settle)
            pick = i;
    at = ring[pick % OPUZ_RING];
    o->next_offset = at.offset;
    o->next_sample = at.sample;
    o->skip = (u32)(target - at.sample);
    c->position = frame;
    return 0;
}

#ifdef OPENDSP_CODEC_BANKS
static Result opuz_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    opuz_data *o = c->data;
    if (!o->dsp)
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (!ogg_backend_context_valid(o->dsp))
        return OPENDSP_AUDIO_ERR_CONTEXT;
    opendspAacStats native = {.struct_size = sizeof(native)};
    Result result = aac_backend_stats(o->dsp, &native);
    if (R_FAILED(result))
        return result;
    memset(stats, 0, sizeof(*stats));
    memcpy(stats, &native, sizeof(native));
    stats->struct_size = sizeof(*stats);
    return 0;
}
#endif

static void opuz_close(audio_codec *c)
{
    opuz_data *o = c->data;
    if (!o)
        return;
#ifdef OPENDSP_CODEC_BANKS
    ogg_backend_park(o->dsp);
#endif
    opendsp_audio_ogg_enter(&o->io.memory);
    opendsp_audio_ogg_release();
    opendsp_audio_ogg_leave();
    free(o->marks);
    free(o);
    c->data = NULL;
}

static const audio_codec_ops opuz_ops = {opuz_read, opuz_seek, opuz_close,
#ifdef OPENDSP_CODEC_BANKS
    opuz_stats
#else
    NULL
#endif
};

static Result read_exact(audio_codec *c, void *data, size_t size)
{
    size_t got;
    Result r = opendsp_audio_input_read(&c->in, data, size, &got);
    if (R_FAILED(r))
        return r;
    return got == size ? 0 : OPENDSP_AUDIO_ERR_TRUNCATED;
}

static Result create_arm(opuz_data *o, u16 channels)
{
    int error = OPUS_OK;
    o->io.error = 0;
    AUDIO_OGG_ENTER(&o->io);
    o->arm = opus_decoder_create(48000, channels, &error);
    opendsp_audio_ogg_leave();
    return o->arm && error == OPUS_OK ? 0 : OPENDSP_AUDIO_ERR_MEMORY;
}

Result opendsp_audio_opuz_open(audio_codec *c, u32 backend)
{
    u8 h[8];
    Result r = read_exact(c, h, sizeof(h));
    if (R_FAILED(r))
        return r;
    if (memcmp(h, "OPUZ", 4))
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (rd32(h + 4))
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    opuz_data *o = calloc(1, sizeof(*o));
    if (!o)
        return OPENDSP_AUDIO_ERR_MEMORY;
    c->data = o;
    c->ops = &opuz_ops;
    o->io.codec = c;
    u32 count = 0, total = 0, repeat_samples = 0, repeat_skip = 0;
    u64 first = 0, repeat_offset = 0;
    bool last = false, repeat = false;
    while (!last && count < OPUZ_RANGES) {
        u8 e[10];
        r = read_exact(c, e, sizeof(e));
        if (R_FAILED(r))
            return r;
        u32 offset = rd32(e), samples = rd32(e + 4), flags = e[8] | e[9] << 8;
        last = flags & 0x8000;
        if (!count) {
            first = offset;
            total = samples;
            o->lookahead = flags & 0x3fff;
        }
        if ((flags & 0x4000) && !repeat) {
            repeat = true;
            repeat_offset = offset;
            repeat_samples = samples;
            repeat_skip = flags & 0x3fff;
        }
        ++count;
    }
    o->data = 8 + (u64)count * 10;
    if (!last || first != o->data || !total || o->data >= c->in.size)
        return OPENDSP_AUDIO_ERR_FORMAT;
    const u8 *data;
    u32 bytes, per_frame;
    u64 next;
    r = packet_at(c, o, o->data, false, &data, &bytes, &per_frame, &next);
    if (R_FAILED(r))
        return r;
    c->info.sample_rate = 48000;
    c->info.channels = (u16)opus_packet_get_nb_channels(data);
    c->info.bits = 16;
    c->info.format = OPENDSP_AUDIO_OPUZ;
    c->info.duration_known = true;
    c->info.total_frames = total;
    c->seekable = true;
    // The repeat range runs to the end.
    if (repeat && repeat_samples && repeat_samples <= total && repeat_offset >= o->data &&
        repeat_offset < c->in.size && o->lookahead + (total - repeat_samples) >= repeat_skip) {
        u64 start = total - repeat_samples;
        o->has_repeat = true;
        o->repeat = (opuz_mark){o->lookahead + start - repeat_skip, repeat_offset};
        c->loop.available = true;
        c->loop.start_frame = start;
        c->loop.end_frame = total;
        c->loop.status = OPENDSP_AUDIO_LOOP_VALID;
    }
    c->backend = OPENDSP_AAC_BACKEND_ARM11;
#ifdef OPENDSP_CODEC_BANKS
    // Same real time limits as Ogg Opus.
    u64 bitrate = (c->in.size - o->data) * 8 * 48000 / total;
    bool fits = per_frame >= 480 && bitrate <= (per_frame < 960 ? 128000u : 288000u);
    if (backend == OPENDSP_AAC_BACKEND_DSP || (backend == OPENDSP_AAC_BACKEND_AUTO && fits)) {
        r = ogg_backend_reuse(&o->dsp, c->info.channels);
        if (R_FAILED(r) && backend == OPENDSP_AAC_BACKEND_DSP)
            return r;
        if (R_FAILED(r))
            o->dsp = NULL;
        if (o->dsp)
            c->backend = OPENDSP_AAC_BACKEND_DSP;
    }
#else
    if (backend == OPENDSP_AAC_BACKEND_DSP) {
        opendsp_codec_error("Opus DSP missing from build", OPENDSP_AUDIO_ERR_UNSUPPORTED);
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    }
#endif
    if (c->backend == OPENDSP_AAC_BACKEND_ARM11) {
        r = create_arm(o, c->info.channels);
        if (R_FAILED(r))
            return r;
    }
    c->memory = opuz_memory(o);
    return opuz_seek(c, 0);
}
