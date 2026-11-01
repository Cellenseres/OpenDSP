// SPDX-License-Identifier: BSD-2-Clause
// NDSP backs up words, not pointers.
#include "voice_state.h"

static vu16 *writer;
static const vu16 *reader;
static void put(u16 value) { *writer++ = value; }
static u16 get(void) { return *reader++; }
static void put32(u32 value) { put((u16)(value >> 16)); put((u16)value); }
static u32 get32(void) { u32 hi = get(); return (hi << 16) | get(); }

static void save_buffer(const wavebuf_t *b, u16 adpcm, u16 queued)
{
    u16 start = queued && !b->played && b->start;
    put32(b->address);
    put32(b->length);
    put(b->id);
    if (!queued) return;
    put(b->looping | (b->played << 1) | (b->adpcm_context << 2) | (start << 3));
    if (adpcm) { put((u16)b->yn1); put((u16)b->yn2); }
    if (start) put32(b->start);
}

static void restore_buffer(wavebuf_t *b, u16 adpcm, u16 queued)
{
    u16 flags;
    b->address = get32();
    b->length = get32();
    b->id = get();
    if (!queued) return;
    flags = get();
    b->looping = flags & 1;
    b->played = (flags >> 1) & 1;
    b->adpcm_context = (flags >> 2) & 1;
    b->yn1 = b->yn2 = 0;
    if (adpcm) { b->yn1 = (s16)get(); b->yn2 = (s16)get(); }
    b->start = (flags & 8) ? get32() : 0;
}

u16 voice_save(const voice_t *v, vu16 *dst)
{
    u16 i, channels = v->stereo ? 2 : 1;
    u16 words = 13 + 4 * channels;
    if (v->format == FMT_ADPCM) words += 2;
    words += v->has_current * 5 + v->queued * (v->format == FMT_ADPCM ? 8 : 6);
    for (i = 0; i < v->queued; ++i)
        if (!v->queue[i].played && v->queue[i].start) words += 2;
    if (v->queued > QUEUE_SIZE || words > VOICE_STATE_WORDS) return 0;
    writer = dst;
    put(v->enabled | (v->format << 1) | (v->stereo << 3) | (v->interp << 4) |
        (v->primed << 6) | (v->head_valid << 7) | (v->queued << 9) |
        (v->has_current << 12) | (v->id_changed << 13));
    put32(v->step);
    put32(v->position);
    put(v->frac);
    for (i = 0; i < channels; ++i) {
        put((u16)v->x0[i]); put((u16)v->x1[i]);
        put((u16)v->h1[i]); put((u16)v->h2[i]);
    }
    put(v->x0_id); put(v->x1_id);
    put32(v->x0_position); put32(v->x1_position);
    put(v->last_valid_id);
    if (v->format == FMT_ADPCM) { put((u16)v->yn1); put((u16)v->yn2); }
    if (v->has_current) save_buffer(&v->current, v->format == FMT_ADPCM, 0);
    for (i = 0; i < v->queued; ++i) save_buffer(&v->queue[i], v->format == FMT_ADPCM, 1);
    return (u16)(writer - dst);
}

void voice_restore(voice_t *v, const vu16 *src)
{
    u16 flags, i, channels, remainder = 0;
    u32 position;
    voice_reset(v);
    reader = src;
    flags = get();
    v->enabled = flags & 1;
    v->format = (flags >> 1) & 3;
    v->stereo = (flags >> 3) & 1;
    v->interp = (flags >> 4) & 3;
    v->primed = (flags >> 6) & 1;
    v->head_valid = (flags >> 7) & 3;
    v->queued = (flags >> 9) & 7;
    v->has_current = (flags >> 12) & 1;
    v->id_changed = (flags >> 13) & 1;
    v->step = get32();
    v->position = get32();
    v->frac = get();
    channels = v->stereo ? 2 : 1;
    for (i = 0; i < channels; ++i) {
        v->x0[i] = (s16)get(); v->x1[i] = (s16)get();
        v->h1[i] = (s16)get(); v->h2[i] = (s16)get();
    }
    v->x0_id = get(); v->x1_id = get();
    v->x0_position = get32(); v->x1_position = get32();
    v->last_valid_id = get();
    if (v->format == FMT_ADPCM) { v->yn1 = (s16)get(); v->yn2 = (s16)get(); }
    if (v->has_current) restore_buffer(&v->current, v->format == FMT_ADPCM, 0);
    for (i = 0; i < v->queued; ++i) restore_buffer(&v->queue[i], v->format == FMT_ADPCM, 1);
    // Reconstruct the ADPCM cursor without division.
    position = v->position;
    for (i = 0; i < 32; ++i) {
        remainder = (remainder << 1) | (u16)(position >> 31);
        position <<= 1;
        v->adpcm_frame <<= 1;
        if (remainder >= 14) { remainder -= 14; v->adpcm_frame |= 1; }
    }
    v->adpcm_nibble = remainder;
}
