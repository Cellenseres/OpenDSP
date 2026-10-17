// SPDX-License-Identifier: BSD-2-Clause
// Inverse quantization, noise substitution, stereo tools, TNS.
#include <string.h>
#include "internal.h"

#define FOR_EACH_BAND(ics, g, sfb, window)                                     \
    for (g = 0, window = 0; g < (ics)->num_groups; window += (ics)->group_len[g], g++) \
        for (sfb = 0; sfb < (ics)->max_sfb; sfb++)

static int32_t dequant_value(int32_t q, int16_t gain, int shift)
{
    int32_t a = q < 0 ? -q : q, m, v;
    if (a > 8191)
        a = 8191;
    if (a <= 1024) {
        m = aac_pow43[a];
    } else {
        // 16 * (j + r/8)^(4/3), interpolated
        int32_t j = a >> 3, r = a & 7;
        m = aac_pow43[j] + (((aac_pow43[j + 1] - aac_pow43[j]) * r) >> 3);
        shift += 4;
    }
    v = shift_sat((int32_t)(((int64_t)m * gain) >> 16), shift);
    return q < 0 ? -v : v;
}

void aac_dequantize(const aac_decoder_t *d, aac_channel_t *c)
{
    const aac_ics_t *ics = &c->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    unsigned g, sfb, window, w, k;
    FOR_EACH_BAND(ics, g, sfb, window) {
        unsigned type = c->band_type[g][sfb];
        int sf;
        int16_t gain;
        int shift;
        if (type == ZERO_HCB || type >= NOISE_HCB)
            continue;
        // q^(4/3) * 2^((sf-100)/4), Q17 times Q14
        sf = c->scalefactor[g][sfb] - 100 + 400;
        gain = aac_pow2_quarter[sf & 3];
        shift = (sf >> 2) - 100 + SPEC_FRAC - 15;
        for (w = 0; w < ics->group_len[g]; w++) {
            int32_t *x = c->spec + (window + w) * 128;
            for (k = swb[sfb]; k < swb[sfb + 1]; k++)
                if (x[k])
                    x[k] = dequant_value(x[k], gain, shift);
        }
    }
}

static void noise_scale(int32_t *dst, const int32_t *rnd, unsigned count, uint32_t energy, int nrg)
{
    unsigned k = 0, i, idx, frac;
    int32_t inv, c;
    int shift;
    if (!energy) {
        memset(dst, 0, count * sizeof(*dst));
        return;
    }
    // energy = m * 4^-k, m >= 2^30
    while (energy < (1u << 30)) {
        energy <<= 2;
        k++;
    }
    idx = (energy >> 26) - 16;
    frac = (energy >> 16) & 1023;
    inv = aac_inv_sqrt[idx] + (((aac_inv_sqrt[idx + 1] - aac_inv_sqrt[idx]) * (int32_t)frac) >> 10);
    // band energy becomes 2^(nrg / 2)
    nrg += 400;
    c = (inv * aac_pow2_quarter[nrg & 3]) >> 14;
    shift = (nrg >> 2) - 100 + (int)k - 16 + SPEC_FRAC - 14;
    for (i = 0; i < count; i++)
        dst[i] = shift_sat(rnd[i] * c, shift);
}

static uint32_t noise_generate(aac_decoder_t *d, int32_t *rnd, unsigned count)
{
    uint32_t energy = 0;
    unsigned i;
    for (i = 0; i < count; i++) {
        d->noise_seed = d->noise_seed * 1664525u + 1013904223u;
        rnd[i] = (int32_t)(d->noise_seed >> 20) - 2048;
        energy += (uint32_t)(rnd[i] * rnd[i]);
    }
    return energy;
}

// noise band pair under M/S
static int shares_noise(const aac_decoder_t *d, const aac_channel_t *other, unsigned g, unsigned sfb)
{
    return other && d->ms_mask_present && d->ms_used[g][sfb] && other->band_type[g][sfb] == NOISE_HCB;
}

static void noise_channel(aac_decoder_t *d, aac_channel_t *c, aac_channel_t *partner, const aac_channel_t *filled)
{
    const aac_ics_t *ics = &c->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    int32_t *rnd = d->fft;
    unsigned g, sfb, window, w;
    FOR_EACH_BAND(ics, g, sfb, window) {
        int shared;
        if (c->band_type[g][sfb] != NOISE_HCB || shares_noise(d, filled, g, sfb))
            continue;
        // same noise in both channels under M/S
        shared = shares_noise(d, partner, g, sfb);
        for (w = 0; w < ics->group_len[g]; w++) {
            unsigned start = (window + w) * 128 + swb[sfb], count = swb[sfb + 1] - swb[sfb];
            uint32_t energy = noise_generate(d, rnd, count);
            noise_scale(c->spec + start, rnd, count, energy, c->scalefactor[g][sfb]);
            if (shared)
                noise_scale(partner->spec + start, rnd, count, energy, partner->scalefactor[g][sfb]);
        }
    }
}

void aac_noise_fill(aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right)
{
    noise_channel(d, left, right, NULL);
    if (right)
        noise_channel(d, right, NULL, left);
}

void aac_mid_side(const aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right)
{
    const aac_ics_t *ics = &left->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    unsigned g, sfb, window, w, k;
    if (!d->ms_mask_present)
        return;
    FOR_EACH_BAND(ics, g, sfb, window) {
        if (!d->ms_used[g][sfb] || left->band_type[g][sfb] >= NOISE_HCB || right->band_type[g][sfb] >= NOISE_HCB)
            continue;
        for (w = 0; w < ics->group_len[g]; w++) {
            int32_t *l = left->spec + (window + w) * 128, *r = right->spec + (window + w) * 128;
            for (k = swb[sfb]; k < swb[sfb + 1]; k++) {
                int32_t m = l[k], s = r[k];
                l[k] = add_sat(m, s);
                r[k] = sat32((int64_t)m - s);
            }
        }
    }
}

void aac_intensity(const aac_decoder_t *d, aac_channel_t *left, aac_channel_t *right)
{
    const aac_ics_t *ics = &right->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    unsigned g, sfb, window, w, k;
    FOR_EACH_BAND(ics, g, sfb, window) {
        unsigned type = right->band_type[g][sfb];
        int invert, position, shift;
        int16_t scale;
        if (type != INTENSITY_HCB && type != INTENSITY_HCB2)
            continue;
        invert = type == INTENSITY_HCB2;
        if (d->ms_mask_present == 1 && d->ms_used[g][sfb])
            invert = !invert;
        // r = l * 0.5^(position / 4)
        position = right->scalefactor[g][sfb] + 400;
        scale = aac_pow2_neg_quarter[position & 3];
        shift = 100 - (position >> 2);
        for (w = 0; w < ics->group_len[g]; w++) {
            const int32_t *l = left->spec + (window + w) * 128;
            int32_t *r = right->spec + (window + w) * 128;
            for (k = swb[sfb]; k < swb[sfb + 1]; k++) {
                int32_t v = shift_sat(mul_q14(l[k], scale), shift);
                r[k] = invert ? -v : v;
            }
        }
    }
}

// all-pole lattice form of the TNS filter
static void tns_filter(int32_t *x, int count, int step, const int16_t *k, unsigned order)
{
    int32_t state[AAC_TNS_MAX_ORDER] = {0};
    unsigned m;
    for (; count > 0; count--, x += step) {
        int32_t f = *x;
        for (m = order; m-- > 0;) {
            f = sat32((int64_t)f - mul_q15(state[m], k[m]));
            if (m + 1 < order)
                state[m + 1] = add_sat(state[m], mul_q15(f, k[m]));
        }
        state[0] = f;
        *x = f;
    }
}

void aac_tns(const aac_decoder_t *d, aac_channel_t *c)
{
    const aac_ics_t *ics = &c->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    int is_short = ics->window_sequence == EIGHT_SHORT_SEQUENCE;
    unsigned limit = is_short ? aac_tns_max_bands_short[d->sf_index] : aac_tns_max_bands_long[d->sf_index];
    unsigned f;
    if (limit > ics->max_sfb)
        limit = ics->max_sfb;
    for (f = 0; f < c->tns_count; f++) {
        const aac_tns_filter_t *t = &c->tns[f];
        unsigned bottom = t->bottom < limit ? t->bottom : limit;
        unsigned top = t->top < limit ? t->top : limit;
        int32_t *x = c->spec + t->window * 128;
        int start = swb[bottom], end = swb[top];
        if (!t->order || end <= start)
            continue;
        if (t->direction)
            tns_filter(x + end - 1, end - start, -1, t->coef, t->order);
        else
            tns_filter(x + start, end - start, 1, t->coef, t->order);
    }
}
