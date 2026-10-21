/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once
#include "hw.h"

enum { MP3_OK, MP3_HISTORY, MP3_TRUNCATED, MP3_CORRUPT, MP3_UNSUPPORTED, MP3_ARGUMENT, MP3_CRC };

typedef struct {
    u16 part_bits, pairs, gain, compress, block, mixed, long_bands, short_bands;
    u16 table[3], region[3], subgain[3], preflag, scale, count1, scfsi;
    const u16 *bands;
} mp3_granule;

/* X: spectrum/work. Leased Y bank 1: state, overlap and tables.
 * Physical sample = stored integer * 2^exponent. */
typedef struct {
    s32 qmf[960];
    u16 reservoir[256];
    mp3_granule gr[4];
    u16 sf[2][40], effective[2][40];
    s32 math[64];
    u16 output[128];
    u16 reservoir_bytes, channels, rate_index, bitrate, frame_bytes, mode;
    u16 bitpos, bitlimit, bad, main_bytes;
    s16 overlap_exp, qmf_exp, exponent;
    s16 power_exp;
    u16 scfsi[2];
    u16 configured;
    u16 stage;
    u16 window_job[8];
    u32 stage_ticks[6], last_stage_ticks[6];
    u16 bits_big_endian;
} mp3_state;
typedef char mp3_bit_state_is_adjacent[__builtin_offsetof(mp3_state, bitlimit) ==
                                                   __builtin_offsetof(mp3_state, bitpos) + sizeof(u16) &&
                                               __builtin_offsetof(mp3_state, bad) ==
                                                   __builtin_offsetof(mp3_state, bitpos) + 2 * sizeof(u16)
                                           ? 1
                                           : -1];

typedef struct {
    mp3_state *s;
    s32 *spectrum;    /* 1152 s32, reuses AAC spectral storage */
    s32 *work;        /* 2112 s32, reuses AAC z/PCM/tmp */
    u16 *overlap[2];  /* 576 words per channel; bank-1 channel 1
                       * is word-aligned, not C s32-aligned */
    const u16 *input; /* packed little-endian byte pairs */
    u16 *main_data;   /* 1024 words in spare overlap-0 space */
    void (*yield)(void);
    void (*emit)(const u16 *, u16); /* 64 sample frames, interleaved PCM16 */
} mp3_decoder;

#ifdef __teak__
void aac_yield(void);
void mp3_codec_yield(void);
void mp3_codec_phase(mp3_state *s, u16 stage);
void mp3_stream_emit(const u16 *pcm, u16 frames);
#define MP3_YIELD(d) mp3_codec_yield()
#define MP3_EMIT(d, p, n) mp3_stream_emit(p, n)
u16 *mp3_pcm_buffer(mp3_decoder *d);
#define MP3_PCM(d) mp3_pcm_buffer(d)
#else
#ifndef MP3_YIELD
#define MP3_YIELD(d) ((d)->yield())
#endif
#ifndef MP3_EMIT
#define MP3_EMIT(d, p, n) ((d)->emit(p, n))
#endif
#define MP3_PCM(d) ((d)->s->output)
#endif

#ifdef __teak__
#define MP3_PHASE(d, n) mp3_codec_phase((d)->s, n)
#else
#define MP3_PHASE(d, n) ((d)->s->stage = (n))
#endif

void mp3_reset(mp3_decoder *d);
u16 mp3_frame(mp3_decoder *d, u16 bytes);
/* Rounded fixed-point product. */
s32 mp3_product(s32 a, s32 b, s16 shift);
#ifdef __teak__
u16 mp3_index(u16 index, u16 stride);
u32 mp3_slot_bytes(u16 index, u16 frames);
u16 mp3_peek_bits(const u16 *data, const u16 *position_limit, u16 bits);
u16 mp3_get_bits(const u16 *data, u16 *position_limit_bad, u16 bits);
void mp3_swap_words(u16 *data, u16 words);
void mp3_align_bytes(u16 *data, u16 words);
#else
static inline u16 mp3_index(u16 index, u16 stride)
{
    return (u16)(index * stride);
}
#endif
#ifdef __teak__
void mp3_dct32(s32 *data, s32 *scratch);
void mp3_window_span(const u16 *job);
void mp3_window_row(const u16 *job);
u16 mp3_requant_small(const u16 *job);
u16 mp3_hybrid_empty(const s32 *data, const u16 *overlap);
void mp3_pcm_span(const u16 *job);
void mp3_synth_window(const u16 *job);
s32 mp3_mul30(s32 a, s32 b);
void mp3_synth_pair(const u16 *job);
void mp3_packed_copy(const u16 *job);
u16 mp3_huff_pairs(const u16 *job);
s16 mp3_maxbits(const void *data, u16 count);
void mp3_rescale(void *data, u16 count, s16 shift);
s16 mp3_scale_pcm(s32 value, s16 shift);
void mp3_alias(s32 *data, s32 *scratch);
void mp3_imdct_long(s32 *data, u16 *overlap, const s32 *window, s32 *scratch);
void mp3_imdct_short(s32 *data, u16 *overlap, const s32 *unused, s32 *scratch);
#endif
