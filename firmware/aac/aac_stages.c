// SPDX-License-Identifier: BSD-2-Clause
// Spectral reconstruction; word-addressed state, high word first.
#include "aac_dsp.h"
#include "aac_fb.h"
#include "aac_tables_dsp.h"

extern s32 mul16(s16 a, s16 b);
u32 aac_lcg(u32 seed);
s32 aac_mul_shift(s32 a, s16 b, s16 shift);
s32 aac_shift_sat(s32 a, s16 shift);
s32 aac_sum_sat(s32 a, s32 b);
s32 aac_diff_sat(s32 a, s32 b);
s32 aac_load32(u16 address);
void aac_store32(u16 address, s32 value);
void clear_words(void *dst, u16 words);

u16 d_overlap[2], d_pcm;
static u32 noise_seed;
static u16 dequant_job[5];
void aac_dequant_span(const u16 *job);
static s32 tns_state[12];

static u16 spectrum(u16 ch) { return (u16)(unsigned)d_spec + (ch ? 2048 : 0); }
static u16 band_offset(u16 ib, u16 sfb)
{
    if (d_ics[ib + I_SEQ] == EIGHT_SHORT) return d_swb_short[sfb];
    return d_swb_long[sfb];
}

void aac_stages_clear(void)
{
    d_prev_shape[0] = d_prev_shape[1] = 0;
    if (d_overlap[0]) {
        clear_words((void *)(unsigned)d_overlap[0], 2048);
        clear_words((void *)(unsigned)d_overlap[1], 2048);
    }
}

void aac_stages_reset(void)
{
    aac_stages_clear();
    noise_seed = 0x2545F491UL;
}

void aac_stage_dequant(u16 ch)
{
    u16 ib = ch ? ICS_WORDS : 0, bb = ch ? BANDS : 0;
    u16 g, sfb, window = 0, w, k, base = spectrum(ch);
    for (g = 0; g < d_ics[ib + I_GROUPS]; g++) {
        u16 glen = d_ics[ib + I_GLEN + g];
        for (sfb = 0; sfb < d_ics[ib + I_MAX_SFB]; sfb++) {
            u16 bi = bb + g * NSFB + sfb, type = d_band_type[bi];
            s16 sf, shift, gain;
            u16 band_begin,band_end;
            if (!type || type >= 13) continue;
            band_begin=band_offset(ib,sfb);
            band_end=band_offset(ib,sfb+1);
            sf = d_scalefactor[bi] + 300;
            gain = daac_pow2_quarter[sf & 3];
            shift = (sf >> 2) - 112;
            aac_yield();
            for (w = 0; w < glen; w++) {
                u16 start = base + ((window + w) << 8);
                for (k = band_begin; k < band_end;) {
                    u16 count = 16 - (k & 15);
                    if (count > band_end - k) count = band_end - k;
                    dequant_job[0] = start + (k << 1);
                    dequant_job[1] = count;
                    dequant_job[2] = (u16)gain;
                    dequant_job[3] = (u16)shift;
                    dequant_job[4] = (u16)(unsigned)daac_pow43;
                    if (!(k & 15)) aac_yield();
                    aac_dequant_span(dequant_job);
                    k += count;
                }
            }
        }
        window += glen;
    }
}

static u32 generate_noise(u16 count)
{
    u16 i;
    u32 energy = 0;
    for (i = 0; i < count; i++) {
        s16 v;
        if (!(i & 31)) aac_yield();
        noise_seed = aac_lcg(noise_seed);
        v = (s16)(noise_seed >> 20) - 2048;
        aac_store32(aac_fb.z + (i << 1), v);
        energy += (u32)mul16(v, v);
    }
    return energy;
}

static void scale_noise(u16 dst, u16 count, u32 energy, s16 nrg)
{
    u16 k = 0, idx, frac, i;
    s32 inv, c;
    s16 shift;
    if (!energy) { clear_words((void *)(unsigned)dst, count << 1); return; }
    while (energy < 0x40000000UL) { energy <<= 2; k++; }
    idx = (u16)(energy >> 26) - 16;
    frac = (u16)(energy >> 16) & 1023;
    inv = daac_inv_sqrt[idx];
    inv += aac_mul_shift((s32)daac_inv_sqrt[idx + 1] - inv, (s16)frac, 10);
    nrg += 400;
    c = aac_mul_shift(inv, daac_pow2_quarter[nrg & 3], 14);
    shift = (nrg >> 2) - 127 + (s16)k;
    for (i = 0; i < count; i++) {
        s16 r = (s16)aac_load32(aac_fb.z + (i << 1));
        if (!(i & 31)) aac_yield();
        aac_store32(dst + (i << 1), aac_shift_sat(aac_mul_shift(c, r, 0), shift));
    }
}

static u16 shares_noise(u16 bi, u16 partner)
{
    u16 other = bi < BANDS ? bi + BANDS : bi - BANDS;
    u16 ms = bi < BANDS ? bi : bi - BANDS;
    if (partner && d_ms_mask_present && d_ms_used[ms] && d_band_type[other] == 13) return 1;
    return 0;
}

void aac_stage_noise(u16 stereo)
{
    u16 ch;
    for (ch = 0; ch < (stereo ? 2 : 1); ch++) {
        u16 ib = ch ? ICS_WORDS : 0, bb = ch ? BANDS : 0;
        u16 g, sfb, w, window = 0;
        for (g = 0; g < d_ics[ib + I_GROUPS]; g++) {
            u16 glen = d_ics[ib + I_GLEN + g];
            for (sfb = 0; sfb < d_ics[ib + I_MAX_SFB]; sfb++) {
                u16 bi = bb + g * NSFB + sfb, shared;
                if (d_band_type[bi] != 13) continue;
                shared = shares_noise(bi, stereo);
                if (ch && shared) continue;
                aac_yield();
                for (w = 0; w < glen; w++) {
                    u16 start = ((window + w) << 7) + band_offset(ib, sfb);
                    u16 count = band_offset(ib, sfb + 1) - band_offset(ib, sfb);
                    u32 energy = generate_noise(count);
                    scale_noise(spectrum(ch) + (start << 1), count, energy, d_scalefactor[bi]);
                    if (shared) scale_noise(spectrum(1) + (start << 1), count, energy, d_scalefactor[bi + BANDS]);
                }
            }
            window += glen;
        }
    }
}

void aac_stage_ms(void)
{
    u16 g, sfb, w, k, window = 0;
    if (!d_ms_mask_present) return;
    for (g = 0; g < d_ics[I_GROUPS]; g++) {
        u16 glen = d_ics[I_GLEN + g];
        for (sfb = 0; sfb < d_ics[I_MAX_SFB]; sfb++) {
            u16 bi = g * NSFB + sfb;
            if (!d_ms_used[bi] || d_band_type[bi] >= 13 || d_band_type[BANDS + bi] >= 13) continue;
            aac_yield();
            for (w = 0; w < glen; w++)
                for (k = band_offset(0, sfb); k < band_offset(0, sfb + 1);) {
                    u16 addr = spectrum(0) + ((window + w) << 8) + (k << 1);
                    u16 count = band_offset(0, sfb + 1) - k;
                    void aac_ms_band(u16 left, u16 right, u16 count);
                    if (count > 32) count = 32;
                    aac_yield();
                    aac_ms_band(addr, addr + 2048, count);
                    k += count;
                }
        }
        window += glen;
    }
}

void aac_stage_intensity(void)
{
    u16 g, sfb, w, k, window = 0, ib = ICS_WORDS;
    for (g = 0; g < d_ics[ib + I_GROUPS]; g++) {
        u16 glen = d_ics[ib + I_GLEN + g];
        for (sfb = 0; sfb < d_ics[ib + I_MAX_SFB]; sfb++) {
            u16 bi = g * NSFB + sfb, type = d_band_type[BANDS + bi], invert = 0;
            s16 position, scale, shift;
            if (type < 14) continue;
            if (type == 14) invert = 1;
            if (d_ms_mask_present == 1 && d_ms_used[bi]) invert ^= 1;
            position = d_scalefactor[BANDS + bi] + 400;
            scale = daac_pow2_neg_quarter[position & 3];
            shift = 100 - (position >> 2);
            aac_yield();
            for (w = 0; w < glen; w++)
                for (k = band_offset(ib, sfb); k < band_offset(ib, sfb + 1); k++) {
                    u16 addr = spectrum(0) + ((window + w) << 8) + (k << 1);
                    s32 v = aac_shift_sat(aac_mul_shift(aac_load32(addr), scale, 14), shift);
                    if (!(k & 15)) aac_yield();
                    if (invert) v = (s32)(0UL - (u32)v);
                    aac_store32(addr + 2048, v);
                }
        }
        window += glen;
    }
}

void aac_stage_tns(u16 ch)
{
    u16 ib = ch ? ICS_WORDS : 0, f, limit;
    limit = d_ics[ib + I_SEQ] == EIGHT_SHORT ? daac_tns_max_bands_short[d_sf_index] : daac_tns_max_bands_long[d_sf_index];
    if (limit > d_ics[ib + I_MAX_SFB]) limit = d_ics[ib + I_MAX_SFB];
    for (f = 0; f < d_tns_count[ch]; f++) {
        u16 tb = ((ch ? 8 : 0) + f) * TNS_WORDS;
        u16 bottom = d_tns[tb + T_BOTTOM], top = d_tns[tb + T_TOP], order = d_tns[tb + T_ORDER];
        u16 start, end, addr, m, count;
        s16 step = 2;
        if (bottom > limit) bottom = limit;
        if (top > limit) top = limit;
        start = band_offset(ib, bottom); end = band_offset(ib, top);
        if (!order || end <= start) continue;
        for (m = 0; m < 12; m++) tns_state[m] = 0;
        addr = spectrum(ch) + (d_tns[tb + T_WINDOW] << 8);
        if (d_tns[tb + T_DIRECTION]) { addr += (end - 1) << 1; step = -2; }
        else addr += start << 1;
        for (count = end - start; count; count--, addr += step) {
            s32 v = aac_load32(addr);
            if (!(count & 7)) aac_yield();
            for (m = order; m-- > 0;) {
                s16 coef = (s16)d_tns[tb + T_COEF + m];
                v = aac_diff_sat(v, aac_mul_shift(tns_state[m], coef, 15));
                if (m + 1 < order) tns_state[m + 1] = aac_sum_sat(tns_state[m], aac_mul_shift(v, coef, 15));
            }
            tns_state[0] = v;
            aac_store32(addr, v);
        }
    }
}

__attribute__((noinline)) void aac_stage_filterbank(u16 ch)
{
    u16 ib = ch ? ICS_WORDS : 0;
    u16 mode = d_ics[ib + I_SEQ] | d_ics[ib + I_SHAPE] << 2 | d_prev_shape[ch] << 3;
    aac_fb_channel(spectrum(ch), d_overlap[ch], d_pcm + (ch ? 1024 : 0), mode);
    d_prev_shape[ch] = d_ics[ib + I_SHAPE];
}

void aac_dsp_stages_sce(void)
{
    aac_stage_dequant(0); aac_stage_noise(0); aac_stage_tns(0); aac_stage_filterbank(0);
}

// A conditional call clobbers flags; never let the compiler emit two in a row.
__attribute__((noinline)) static void stereo_tools(void)
{
    aac_stage_ms();
    aac_stage_intensity();
}

void aac_dsp_stages_cpe(u16 common_window)
{
    aac_stage_dequant(0); aac_stage_dequant(1); aac_stage_noise(1);
    if (common_window) stereo_tools();
    aac_stage_tns(0); aac_stage_tns(1); aac_stage_filterbank(0); aac_stage_filterbank(1);
}
