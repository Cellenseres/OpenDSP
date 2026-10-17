// SPDX-License-Identifier: BSD-2-Clause
// AAC core helpers: bit reader, fixed-point math.
#pragma once
#include "aac.h"
#include "tables.h"

#define ONLY_LONG_SEQUENCE   0
#define LONG_START_SEQUENCE  1
#define EIGHT_SHORT_SEQUENCE 2
#define LONG_STOP_SEQUENCE   3

#define ZERO_HCB       0
#define ESC_HCB        11
#define NOISE_HCB      13
#define INTENSITY_HCB2 14
#define INTENSITY_HCB  15

// fraction bits: spectrum 3, time samples 8
#define SPEC_FRAC 3
#define TIME_FRAC 8

typedef struct {
    const uint8_t *buf;
    uint32_t bytes;
    uint32_t pos;       // in bits
} aac_bits_t;

static inline uint32_t bits_window(const aac_bits_t *b)
{
    uint32_t i = b->pos >> 3, w = 0, k;
    if (i + 4 <= b->bytes) {
        w = (uint32_t)b->buf[i] << 24 | (uint32_t)b->buf[i + 1] << 16 |
            (uint32_t)b->buf[i + 2] << 8 | b->buf[i + 3];
    } else {
        for (k = 0; k < 4; k++)
            w = w << 8 | (i + k < b->bytes ? b->buf[i + k] : 0);
    }
    return w << (b->pos & 7);
}

// n = 1..25
static inline uint32_t bits_peek(const aac_bits_t *b, unsigned n)
{
    return bits_window(b) >> (32 - n);
}

static inline uint32_t bits_read(aac_bits_t *b, unsigned n)
{
    uint32_t v = bits_peek(b, n);
    b->pos += n;
    return v;
}

static inline unsigned bits_read1(aac_bits_t *b)
{
    return bits_read(b, 1);
}

static inline void bits_align(aac_bits_t *b)
{
    b->pos = (b->pos + 7) & ~7u;
}

static inline int bits_overrun(const aac_bits_t *b)
{
    return b->pos > b->bytes * 8;
}

// leaf len<<11|sym, node 0x8000|bits<<11|offset
static inline unsigned huff_decode(aac_bits_t *b, const uint16_t *table, unsigned root)
{
    unsigned bits = root;
    uint16_t e = table[bits_peek(b, bits)];
    while (e & 0x8000) {
        b->pos += bits;
        bits = (e >> 11) & 15;
        e = table[(e & 0x7FF) + bits_peek(b, bits)];
    }
    b->pos += e >> 11;
    return e & 0x7FF;
}

// 32x16 multiplies
static inline int32_t mul_q15(int32_t a, int16_t b)
{
    return (int32_t)(((int64_t)a * b) >> 15);
}

static inline int32_t mul_q14(int32_t a, int16_t b)
{
    return (int32_t)(((int64_t)a * b) >> 14);
}

static inline int32_t sat32(int64_t v)
{
    return v > INT32_MAX ? INT32_MAX : v < -INT32_MAX ? -INT32_MAX : (int32_t)v;
}

static inline int32_t add_sat(int32_t a, int32_t b)
{
    return sat32((int64_t)a + b);
}

// positive: saturating left, negative: rounding right
static inline int32_t shift_sat(int32_t v, int shift)
{
    if (shift >= 0) {
        if (shift > 31)
            return v > 0 ? INT32_MAX : v < 0 ? -INT32_MAX : 0;
        return sat32((int64_t)v * ((int64_t)1 << shift));
    }
    shift = -shift;
    if (shift > 31)
        return 0;
    return (int32_t)(((int64_t)v + ((int64_t)1 << (shift - 1))) >> shift);
}

static inline unsigned clz32(uint32_t v)
{
    unsigned n = 0;
    if (!v)
        return 32;
    while (!(v & 0x80000000u)) {
        v <<= 1;
        n++;
    }
    return n;
}

static inline const uint16_t *swb_offsets(const aac_decoder_t *d, const aac_ics_t *ics)
{
    return ics->window_sequence == EIGHT_SHORT_SEQUENCE ? aac_swb_short[d->sf_index] : aac_swb_long[d->sf_index];
}

// syntax.c
aac_status_t aac_decode_raw_block(aac_decoder_t *d, aac_bits_t *b, int16_t *pcm);

// spectrum.c
void aac_dequantize(const aac_decoder_t *d, aac_channel_t *c);
void aac_noise_fill(aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right);
void aac_mid_side(const aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right);
void aac_intensity(const aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right);
void aac_tns(const aac_decoder_t *d, aac_channel_t *c);

// filterbank.c
void aac_filterbank(aac_decoder_t *d, unsigned ch, int16_t *pcm, unsigned stride);
