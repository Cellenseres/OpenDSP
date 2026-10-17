// SPDX-License-Identifier: BSD-2-Clause
// raw_data_block parsing: elements, side info, Huffman spectral data.
#include <string.h>
#include "internal.h"

#define ID_SCE 0
#define ID_CPE 1
#define ID_CCE 2
#define ID_LFE 3
#define ID_DSE 4
#define ID_PCE 5
#define ID_FIL 6
#define ID_END 7

static aac_status_t parse_ics_info(const aac_decoder_t *d, aac_bits_t *b, aac_ics_t *ics)
{
    unsigned i;
    if (bits_read1(b))
        return AAC_CORRUPT;
    ics->window_sequence = (uint8_t)bits_read(b, 2);
    ics->window_shape = (uint8_t)bits_read1(b);
    if (ics->window_sequence == EIGHT_SHORT_SEQUENCE) {
        unsigned grouping;
        ics->max_sfb = (uint8_t)bits_read(b, 4);
        grouping = bits_read(b, 7);
        ics->num_swb = aac_num_swb_short[d->sf_index];
        ics->num_windows = 8;
        ics->num_groups = 1;
        ics->group_len[0] = 1;
        for (i = 0; i < 7; i++) {
            if (grouping & (0x40 >> i)) {
                ics->group_len[ics->num_groups - 1]++;
            } else {
                ics->group_len[ics->num_groups] = 1;
                ics->num_groups++;
            }
        }
    } else {
        ics->max_sfb = (uint8_t)bits_read(b, 6);
        // prediction exists only in AAC Main
        if (bits_read1(b))
            return AAC_CORRUPT;
        ics->num_swb = aac_num_swb_long[d->sf_index];
        ics->num_windows = 1;
        ics->num_groups = 1;
        ics->group_len[0] = 1;
    }
    return ics->max_sfb > ics->num_swb ? AAC_CORRUPT : AAC_OK;
}

static aac_status_t parse_section_data(aac_bits_t *b, aac_channel_t *c, int allow_intensity)
{
    const aac_ics_t *ics = &c->ics;
    unsigned bits = ics->window_sequence == EIGHT_SHORT_SEQUENCE ? 3 : 5;
    unsigned escape = (1u << bits) - 1, g;
    memset(c->band_type, 0, sizeof(c->band_type));
    for (g = 0; g < ics->num_groups; g++) {
        unsigned k = 0;
        while (k < ics->max_sfb) {
            unsigned type = bits_read(b, 4), len = 0, incr, end;
            if (type == 12 || (!allow_intensity && type >= INTENSITY_HCB2))
                return AAC_CORRUPT;
            while ((incr = bits_read(b, bits)) == escape) {
                len += escape;
                if (len > ics->max_sfb)
                    return AAC_CORRUPT;
            }
            len += incr;
            end = k + len;
            if (!len || end > ics->max_sfb)
                return AAC_CORRUPT;
            for (; k < end; k++)
                c->band_type[g][k] = (uint8_t)type;
        }
    }
    return AAC_OK;
}

static aac_status_t parse_scalefactors(aac_bits_t *b, aac_channel_t *c, unsigned global_gain)
{
    const aac_ics_t *ics = &c->ics;
    int scale = (int)global_gain, noise = (int)global_gain - 90, position = 0, first_noise = 1;
    unsigned g, sfb;
    for (g = 0; g < ics->num_groups; g++) {
        for (sfb = 0; sfb < ics->max_sfb; sfb++) {
            unsigned type = c->band_type[g][sfb];
            int value = 0;
            if (type == ZERO_HCB) {
                value = 0;
            } else if (type == INTENSITY_HCB || type == INTENSITY_HCB2) {
                position += (int)huff_decode(b, aac_huff_tables[0], aac_huff_root_bits[0]) - 60;
                if (position < -255 || position > 255)
                    return AAC_CORRUPT;
                value = position;
            } else if (type == NOISE_HCB) {
                if (first_noise) {
                    first_noise = 0;
                    noise += (int)bits_read(b, 9) - 256;
                } else {
                    noise += (int)huff_decode(b, aac_huff_tables[0], aac_huff_root_bits[0]) - 60;
                }
                if (noise < -255 || noise > 255)
                    return AAC_CORRUPT;
                value = noise;
            } else {
                scale += (int)huff_decode(b, aac_huff_tables[0], aac_huff_root_bits[0]) - 60;
                if (scale < 0 || scale > 255)
                    return AAC_CORRUPT;
                value = scale;
            }
            c->scalefactor[g][sfb] = (int16_t)value;
        }
    }
    return AAC_OK;
}

static aac_status_t parse_tns(aac_bits_t *b, aac_channel_t *c)
{
    const aac_ics_t *ics = &c->ics;
    int is_short = ics->window_sequence == EIGHT_SHORT_SEQUENCE;
    unsigned max_order = is_short ? 7 : AAC_TNS_MAX_ORDER;
    unsigned w, f, i;
    c->tns_count = 0;
    for (w = 0; w < ics->num_windows; w++) {
        unsigned n_filt = bits_read(b, is_short ? 1 : 2), top = ics->num_swb, res = 0;
        if (n_filt)
            res = bits_read1(b) + 3;
        for (f = 0; f < n_filt; f++) {
            aac_tns_filter_t *t = &c->tns[c->tns_count];
            unsigned length = bits_read(b, is_short ? 4 : 6);
            unsigned order = bits_read(b, is_short ? 3 : 5);
            if (c->tns_count >= AAC_MAX_TNS_FILTERS || order > max_order)
                return AAC_CORRUPT;
            t->window = (uint8_t)w;
            t->top = (uint8_t)top;
            t->bottom = (uint8_t)(length < top ? top - length : 0);
            t->order = (uint8_t)order;
            top = t->bottom;
            if (order) {
                unsigned compress, bits;
                const int16_t *table = res == 4 ? aac_tns_coef_4bit : aac_tns_coef_3bit;
                int half = 1 << (res - 1);
                t->direction = (uint8_t)bits_read1(b);
                compress = bits_read1(b);
                bits = res - compress;
                for (i = 0; i < order; i++) {
                    int v = (int)bits_read(b, bits);
                    if (v & (1 << (bits - 1)))
                        v -= 1 << bits;
                    t->coef[i] = table[v + half];
                }
            }
            c->tns_count++;
        }
    }
    return AAC_OK;
}

static int read_escape(aac_bits_t *b, int value)
{
    unsigned n = 0;
    if (value != 16)
        return value;
    while (bits_read1(b)) {
        if (++n > 8)
            return -1;
    }
    return (1 << (n + 4)) + (int)bits_read(b, n + 4);
}

static aac_status_t decode_band(aac_bits_t *b, unsigned type, int32_t *dst, unsigned count)
{
    const uint16_t *table = aac_huff_tables[type];
    unsigned root = aac_huff_root_bits[type], i;
    if (type <= 4) {
        for (i = 0; i < count; i += 4) {
            unsigned sym = huff_decode(b, table, root), j;
            for (j = 0; j < 4; j++) {
                int v = (int)((sym >> (6 - 2 * j)) & 3);
                if (type <= 2)
                    v -= 1;
                else if (v && bits_read1(b))
                    v = -v;
                dst[i + j] = v;
            }
        }
        return AAC_OK;
    }
    for (i = 0; i < count; i += 2) {
        unsigned sym = huff_decode(b, table, root);
        int y, z;
        if (type == ESC_HCB) {
            y = (int)(sym >> 5);
            z = (int)(sym & 31);
        } else {
            y = (int)(sym >> 4);
            z = (int)(sym & 15);
        }
        if (type <= 6) {
            y -= 4;
            z -= 4;
        } else {
            int sy = y && bits_read1(b);
            int sz = z && bits_read1(b);
            if (type == ESC_HCB) {
                y = read_escape(b, y);
                z = read_escape(b, z);
                if (y < 0 || z < 0)
                    return AAC_CORRUPT;
            }
            if (sy)
                y = -y;
            if (sz)
                z = -z;
        }
        dst[i] = y;
        dst[i + 1] = z;
    }
    return AAC_OK;
}

static aac_status_t parse_spectral_data(const aac_decoder_t *d, aac_bits_t *b, aac_channel_t *c)
{
    const aac_ics_t *ics = &c->ics;
    const uint16_t *swb = swb_offsets(d, ics);
    unsigned g, sfb, w, window = 0;
    memset(c->spec, 0, sizeof(c->spec));
    for (g = 0; g < ics->num_groups; g++) {
        for (sfb = 0; sfb < ics->max_sfb; sfb++) {
            unsigned type = c->band_type[g][sfb];
            if (type == ZERO_HCB || type >= NOISE_HCB)
                continue;
            for (w = 0; w < ics->group_len[g]; w++) {
                int32_t *dst = c->spec + (window + w) * 128 + swb[sfb];
                aac_status_t s = decode_band(b, type, dst, swb[sfb + 1] - swb[sfb]);
                if (s != AAC_OK)
                    return s;
            }
        }
        window += ics->group_len[g];
    }
    return AAC_OK;
}

static aac_status_t parse_pulse(aac_bits_t *b, const aac_channel_t *c, uint8_t *pulse)
{
    unsigned count = bits_read(b, 2) + 1, start = bits_read(b, 6), i;
    if (c->ics.window_sequence == EIGHT_SHORT_SEQUENCE || start >= c->ics.num_swb)
        return AAC_CORRUPT;
    pulse[0] = (uint8_t)count;
    pulse[1] = (uint8_t)start;
    for (i = 0; i < count; i++) {
        pulse[2 + 2 * i] = (uint8_t)bits_read(b, 5);
        pulse[3 + 2 * i] = (uint8_t)bits_read(b, 4);
    }
    return AAC_OK;
}

static aac_status_t apply_pulse(const aac_decoder_t *d, aac_channel_t *c, const uint8_t *pulse)
{
    unsigned k = aac_swb_long[d->sf_index][pulse[1]], i;
    for (i = 0; i < pulse[0]; i++) {
        int32_t amp = pulse[3 + 2 * i];
        k += pulse[2 + 2 * i];
        if (k >= AAC_FRAME_SAMPLES)
            return AAC_CORRUPT;
        c->spec[k] += c->spec[k] > 0 ? amp : -amp;
    }
    return AAC_OK;
}

static aac_status_t parse_ics(aac_decoder_t *d, aac_bits_t *b, aac_channel_t *c, int common_window,
                              int allow_intensity)
{
    unsigned global_gain = bits_read(b, 8);
    uint8_t pulse[10];
    int pulse_present, tns_present;
    aac_status_t s;
    if (!common_window && (s = parse_ics_info(d, b, &c->ics)) != AAC_OK)
        return s;
    if ((s = parse_section_data(b, c, allow_intensity)) != AAC_OK)
        return s;
    if ((s = parse_scalefactors(b, c, global_gain)) != AAC_OK)
        return s;
    pulse_present = bits_read1(b);
    if (pulse_present && (s = parse_pulse(b, c, pulse)) != AAC_OK)
        return s;
    tns_present = bits_read1(b);
    c->tns_count = 0;
    if (tns_present && (s = parse_tns(b, c)) != AAC_OK)
        return s;
    // gain control is AAC SSR only
    if (bits_read1(b))
        return AAC_UNSUPPORTED;
    if (bits_overrun(b))
        return AAC_CORRUPT;
    if ((s = parse_spectral_data(d, b, c)) != AAC_OK)
        return s;
    if (pulse_present && (s = apply_pulse(d, c, pulse)) != AAC_OK)
        return s;
    return bits_overrun(b) ? AAC_CORRUPT : AAC_OK;
}

static aac_status_t decode_sce(aac_decoder_t *d, aac_bits_t *b, int16_t *pcm)
{
    aac_channel_t *c = &d->ch[0];
    aac_status_t s = parse_ics(d, b, c, 0, 0);
    if (s != AAC_OK)
        return s;
    aac_dequantize(d, c);
    aac_noise_fill(d, c, NULL);
    aac_tns(d, c);
    aac_filterbank(d, 0, pcm, 1);
    return AAC_OK;
}

static aac_status_t decode_cpe(aac_decoder_t *d, aac_bits_t *b, int16_t *pcm)
{
    aac_channel_t *l = &d->ch[0], *r = &d->ch[1];
    int common_window = bits_read1(b);
    aac_status_t s;
    d->ms_mask_present = 0;
    if (common_window) {
        unsigned g, sfb;
        if ((s = parse_ics_info(d, b, &l->ics)) != AAC_OK)
            return s;
        r->ics = l->ics;
        d->ms_mask_present = (uint8_t)bits_read(b, 2);
        if (d->ms_mask_present == 3)
            return AAC_CORRUPT;
        for (g = 0; g < l->ics.num_groups; g++)
            for (sfb = 0; sfb < l->ics.max_sfb; sfb++)
                d->ms_used[g][sfb] = (uint8_t)(d->ms_mask_present == 2 ? 1 : d->ms_mask_present ? bits_read1(b) : 0);
    }
    if ((s = parse_ics(d, b, l, common_window, 0)) != AAC_OK)
        return s;
    if ((s = parse_ics(d, b, r, common_window, common_window)) != AAC_OK)
        return s;
    aac_dequantize(d, l);
    aac_dequantize(d, r);
    aac_noise_fill(d, l, r);
    if (common_window) {
        aac_mid_side(d, l, r);
        aac_intensity(d, l, r);
    }
    aac_tns(d, l);
    aac_tns(d, r);
    aac_filterbank(d, 0, pcm, 2);
    aac_filterbank(d, 1, pcm + 1, 2);
    return AAC_OK;
}

static void skip_bytes(aac_bits_t *b, unsigned count)
{
    b->pos += count * 8;
}

static void skip_dse(aac_bits_t *b)
{
    unsigned align, count;
    bits_read(b, 4);
    align = bits_read1(b);
    count = bits_read(b, 8);
    if (count == 255)
        count += bits_read(b, 8);
    if (align)
        bits_align(b);
    skip_bytes(b, count);
}

#define EXT_SBR_DATA     13
#define EXT_SBR_DATA_CRC 14

static void skip_fil(aac_decoder_t *d, aac_bits_t *b)
{
    unsigned count = bits_read(b, 4), type;
    if (count == 15)
        count += bits_read(b, 8) - 1;
    type = count ? bits_peek(b, 4) : 0;
    if (type == EXT_SBR_DATA || type == EXT_SBR_DATA_CRC)
        d->sbr_seen = 1;
    skip_bytes(b, count);
}

static void skip_pce(aac_bits_t *b)
{
    unsigned front, side, back, lfe, assoc, cc;
    bits_read(b, 10);   // tag, object type, sampling index
    front = bits_read(b, 4);
    side = bits_read(b, 4);
    back = bits_read(b, 4);
    lfe = bits_read(b, 2);
    assoc = bits_read(b, 3);
    cc = bits_read(b, 4);
    if (bits_read1(b))
        bits_read(b, 4);
    if (bits_read1(b))
        bits_read(b, 4);
    if (bits_read1(b))
        bits_read(b, 3);
    b->pos += (front + side + back) * 5 + lfe * 4 + assoc * 4 + cc * 5;
    bits_align(b);
    skip_bytes(b, bits_read(b, 8));
}

aac_status_t aac_decode_raw_block(aac_decoder_t *d, aac_bits_t *b, int16_t *pcm)
{
    unsigned audio = 0;
    for (;;) {
        unsigned id = bits_read(b, 3);
        aac_status_t s = AAC_OK;
        if (bits_overrun(b))
            return AAC_CORRUPT;
        switch (id) {
        case ID_SCE:
        case ID_CPE:
            if (audio || (id == ID_SCE) != (d->channels == 1))
                return AAC_CORRUPT;
            bits_read(b, 4);
            s = id == ID_SCE ? decode_sce(d, b, pcm) : decode_cpe(d, b, pcm);
            audio = 1;
            break;
        case ID_CCE:
        case ID_LFE:
            return AAC_UNSUPPORTED;
        case ID_DSE:
            skip_dse(b);
            break;
        case ID_PCE:
            skip_pce(b);
            break;
        case ID_FIL:
            skip_fil(d, b);
            break;
        default:
            bits_align(b);
            if (bits_overrun(b))
                return AAC_CORRUPT;
            return audio ? AAC_OK : AAC_CORRUPT;
        }
        if (s != AAC_OK)
            return s;
    }
}
