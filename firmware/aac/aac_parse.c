// SPDX-License-Identifier: BSD-2-Clause
// ADTS frames and raw_data_block parsing on the DSP; mirrors decoder.c and
// syntax.c of the ARM11 decoder. teak-llvm rules: u16 indices into flat
// global arrays, at most four arguments, no local arrays.
#include "aac_dsp.h"
#include "aac_tables_dsp.h"

#define ZERO_HCB 0
#define ESC_HCB 11
#define NOISE_HCB 13
#define INTENSITY_HCB2 14
#define INTENSITY_HCB 15

u16 aac_bs_data[BS_WORDS + 2] __attribute__((section(".aac_input")));
u16 aac_bs_bits;
u16 d_ics[2 * ICS_WORDS];
u16 d_band_type[2 * BANDS] __attribute__((section(".aac_types")));
s16 d_scalefactor[2 * BANDS] __attribute__((section(".aac_sf")));
u16 d_ms_used[BANDS] __attribute__((section(".aac_ms")));
u16 d_tns[2 * 8 * TNS_WORDS];
u16 d_tns_count[2];
u16 d_spec[2 * 2048];
u16 d_swb_long[52], d_swb_short[16];
u16 d_ms_mask_present, d_sf_index, d_channels, d_configured, d_synced, d_sbr_seen;
u16 d_prev_shape[2];

static u16 huff_table[12];              // word addresses
static u16 pulse[10];

void clear_words(void *dst, u16 words);

void aac_dsp_stages_sce(void);
void aac_dsp_stages_cpe(u16 common_window);

static u16 overrun(void)
{
    if (aac_bs_pos > aac_bs_bits)
        return 1;
    return 0;
}

static void align(void)
{
    aac_bs_pos = (aac_bs_pos + 7) & 0xFFF8;
}

static void copy_table(u16 *dst, const u16 *src, u16 count)
{
    u16 i;
    for (i = 0; i < count; i++)
        dst[i] = src[i];
}

// active band offsets for this sampling rate
static void select_rate(u16 sf)
{
    if (sf <= 1) {
        copy_table(d_swb_long, daac_swb_long_96, 42);
        copy_table(d_swb_short, daac_swb_short_96, 13);
    } else if (sf == 2) {
        copy_table(d_swb_long, daac_swb_long_64, 48);
        copy_table(d_swb_short, daac_swb_short_96, 13);
    } else if (sf <= 4) {
        copy_table(d_swb_long, daac_swb_long_48, 50);
        copy_table(d_swb_short, daac_swb_short_48, 15);
    } else if (sf == 5) {
        copy_table(d_swb_long, daac_swb_long_32, 52);
        copy_table(d_swb_short, daac_swb_short_48, 15);
    } else if (sf <= 7) {
        copy_table(d_swb_long, daac_swb_long_24, 48);
        copy_table(d_swb_short, daac_swb_short_24, 16);
    } else if (sf <= 10) {
        copy_table(d_swb_long, daac_swb_long_16, 44);
        copy_table(d_swb_short, daac_swb_short_16, 16);
    } else {
        copy_table(d_swb_long, daac_swb_long_8, 41);
        copy_table(d_swb_short, daac_swb_short_8, 16);
    }
}

void aac_dsp_init(void)
{
    AAC_BS_BASE = (u16)(unsigned)aac_bs_data;
    huff_table[0] = (u16)(unsigned)daac_huff_sf;
    huff_table[1] = (u16)(unsigned)daac_huff_1;
    huff_table[2] = (u16)(unsigned)daac_huff_2;
    huff_table[3] = (u16)(unsigned)daac_huff_3;
    huff_table[4] = (u16)(unsigned)daac_huff_4;
    huff_table[5] = (u16)(unsigned)daac_huff_5;
    huff_table[6] = (u16)(unsigned)daac_huff_6;
    huff_table[7] = (u16)(unsigned)daac_huff_7;
    huff_table[8] = (u16)(unsigned)daac_huff_8;
    huff_table[9] = (u16)(unsigned)daac_huff_9;
    huff_table[10] = (u16)(unsigned)daac_huff_10;
    huff_table[11] = (u16)(unsigned)daac_huff_11;
    aac_dsp_reset();
}

static u16 ics_base(u16 ch)
{
    return ch ? ICS_WORDS : 0;
}

static u16 parse_ics_info(u16 ch)
{
    u16 ib = ics_base(ch), i, grouping, groups;
    if (aac_getbits(1))
        return AAC_CORRUPT;
    d_ics[ib + I_SEQ] = aac_getbits(2);
    d_ics[ib + I_SHAPE] = aac_getbits(1);
    if (d_ics[ib + I_SEQ] == EIGHT_SHORT) {
        d_ics[ib + I_MAX_SFB] = aac_getbits(4);
        grouping = aac_getbits(7);
        d_ics[ib + I_NUM_SWB] = daac_num_swb_short[d_sf_index];
        d_ics[ib + I_WINDOWS] = 8;
        groups = 1;
        d_ics[ib + I_GLEN] = 1;
        for (i = 0; i < 7; i++) {
            if (grouping & (0x40 >> i)) {
                d_ics[ib + I_GLEN + groups - 1]++;
            } else {
                d_ics[ib + I_GLEN + groups] = 1;
                groups++;
            }
        }
        d_ics[ib + I_GROUPS] = groups;
    } else {
        d_ics[ib + I_MAX_SFB] = aac_getbits(6);
        if (aac_getbits(1))             // prediction: AAC Main only
            return AAC_CORRUPT;
        d_ics[ib + I_NUM_SWB] = daac_num_swb_long[d_sf_index];
        d_ics[ib + I_WINDOWS] = 1;
        d_ics[ib + I_GROUPS] = 1;
        d_ics[ib + I_GLEN] = 1;
    }
    if (d_ics[ib + I_MAX_SFB] > d_ics[ib + I_NUM_SWB])
        return AAC_CORRUPT;
    return AAC_OK;
}

static u16 parse_section_data(u16 ch, u16 allow_intensity)
{
    u16 ib = ics_base(ch), bb = ch ? BANDS : 0, g, gb;
    u16 max_sfb = d_ics[ib + I_MAX_SFB], groups = d_ics[ib + I_GROUPS];
    u16 bits = d_ics[ib + I_SEQ] == EIGHT_SHORT ? 3 : 5;
    u16 escape = (1u << bits) - 1;
    clear_words((void *)(unsigned)((u16)(unsigned)d_band_type + bb), BANDS);
    for (g = 0, gb = bb; g < groups; g++, gb += NSFB) {
        u16 k = 0;
        while (k < max_sfb) {
            u16 type = aac_getbits(4), len = 0, incr, end;
            aac_yield();
            if (type == 12)
                return AAC_CORRUPT;
            if (!allow_intensity && type >= INTENSITY_HCB2)
                return AAC_CORRUPT;
            for (;;) {
                incr = aac_getbits(bits);
                if (incr != escape)
                    break;
                len += escape;
                if (len > max_sfb)
                    return AAC_CORRUPT;
            }
            len += incr;
            end = k + len;
            if (!len || end > max_sfb)
                return AAC_CORRUPT;
            for (; k < end; k++)
                d_band_type[gb + k] = type;
        }
    }
    return AAC_OK;
}

static u16 parse_scalefactors(u16 ch, u16 global_gain)
{
    u16 ib = ics_base(ch), bb = ch ? BANDS : 0, g, gb, sfb, first_noise = 1;
    u16 max_sfb = d_ics[ib + I_MAX_SFB], groups = d_ics[ib + I_GROUPS];
    s16 scale = (s16)global_gain, noise = (s16)global_gain - 90, position = 0;
    for (g = 0, gb = bb; g < groups; g++, gb += NSFB) {
        for (sfb = 0; sfb < max_sfb; sfb++) {
            aac_yield();
            u16 type = d_band_type[gb + sfb];
            s16 value = 0;
            if (type == ZERO_HCB) {
                value = 0;
            } else if (type >= INTENSITY_HCB2) {
                position += (s16)aac_huff(huff_table[0]) - 60;
                if (position < -255 || position > 255)
                    return AAC_CORRUPT;
                value = position;
            } else if (type == NOISE_HCB) {
                if (first_noise) {
                    first_noise = 0;
                    noise += (s16)aac_getbits(9) - 256;
                } else {
                    noise += (s16)aac_huff(huff_table[0]) - 60;
                }
                if (noise < -255 || noise > 255)
                    return AAC_CORRUPT;
                value = noise;
            } else {
                scale += (s16)aac_huff(huff_table[0]) - 60;
                if (scale < 0 || scale > 255)
                    return AAC_CORRUPT;
                value = scale;
            }
            d_scalefactor[gb + sfb] = value;
        }
    }
    return AAC_OK;
}

static u16 parse_tns(u16 ch)
{
    u16 ib = ics_base(ch), w, f, i, count = 0;
    u16 is_short = d_ics[ib + I_SEQ] == EIGHT_SHORT, max_order = is_short ? 7 : 12;
    u16 windows = d_ics[ib + I_WINDOWS];
    for (w = 0; w < windows; w++) {
        u16 n_filt = aac_getbits(is_short ? 1 : 2), top = d_ics[ib + I_NUM_SWB], res = 0;
        if (n_filt)
            res = aac_getbits(1) + 3;
        for (f = 0; f < n_filt; f++) {
            u16 length = aac_getbits(is_short ? 4 : 6);
            u16 order = aac_getbits(is_short ? 3 : 5);
            u16 tb = ((ch ? 8 : 0) + count) * TNS_WORDS;
            if (count >= 8 || order > max_order)
                return AAC_CORRUPT;
            d_tns[tb + T_WINDOW] = w;
            d_tns[tb + T_TOP] = top;
            d_tns[tb + T_BOTTOM] = length < top ? top - length : 0;
            d_tns[tb + T_ORDER] = order;
            top = d_tns[tb + T_BOTTOM];
            if (order) {
                u16 compress, bits, half = res == 4 ? 8 : 4;
                d_tns[tb + T_DIRECTION] = aac_getbits(1);
                compress = aac_getbits(1);
                bits = res - compress;
                for (i = 0; i < order; i++) {
                    u16 v = aac_getbits(bits);
                    if (v & (1u << (bits - 1)))
                        v -= 1u << bits;    // two's complement in 16 bits
                    v += half;
                    d_tns[tb + T_COEF + i] = (u16)(res == 4 ? daac_tns_coef_4bit[v & 15] : daac_tns_coef_3bit[v & 7]);
                }
            }
            count++;
        }
    }
    d_tns_count[ch] = count;
    return AAC_OK;
}

// s32 store, high word first; inlined into the band loops it crashes teak-llvm
__attribute__((noinline)) static void put32(u16 addr, s16 v)
{
    u16 *p = (u16 *)(unsigned)addr;
    p[0] = v < 0 ? 0xFFFF : 0;
    p[1] = (u16)v;
}

// v negated when s = 1. Branch-free: teak-llvm reloads values between the
// compare and a conditional neg, and the reload clobbers the flags.
static s16 apply_sign(s16 v, u16 s)
{
    return (s16)(((u16)v ^ (u16)(0 - s)) + s);
}

static u16 band_job[4];

static u16 parse_spectral_data(u16 ch)
{
    u16 ib = ics_base(ch), bb = ch ? BANDS : 0, g, gb, sfb, w, window = 0;
    u16 spec = (u16)(unsigned)d_spec + (ch ? 2048 : 0);
    u16 max_sfb = d_ics[ib + I_MAX_SFB], groups = d_ics[ib + I_GROUPS];
    u16 is_short = d_ics[ib + I_SEQ] == EIGHT_SHORT;
    clear_words((void *)(unsigned)spec, 2048);
    for (g = 0, gb = bb; g < groups; g++, gb += NSFB) {
        u16 glen = d_ics[ib + I_GLEN + g];
        for (sfb = 0; sfb < max_sfb; sfb++) {
            u16 type = d_band_type[gb + sfb], start, width;
            if (type == ZERO_HCB || type >= NOISE_HCB)
                continue;
            start = is_short ? d_swb_short[sfb] : d_swb_long[sfb];
            width = (is_short ? d_swb_short[sfb + 1] : d_swb_long[sfb + 1]) - start;
            for (w = 0; w < glen; w++) {
                band_job[0] = type;
                band_job[1] = huff_table[type];
                band_job[2] = spec + ((((window + w) << 7) + start) << 1);
                band_job[3] = width;
                aac_yield();
                if (aac_decode_band(band_job))
                    return AAC_CORRUPT;
            }
        }
        window += glen;
    }
    return AAC_OK;
}

static u16 parse_pulse(u16 ch)
{
    u16 count = aac_getbits(2) + 1, start = aac_getbits(6), i, ib = ics_base(ch);
    if (d_ics[ib + I_SEQ] == EIGHT_SHORT || start >= d_ics[ib + I_NUM_SWB])
        return AAC_CORRUPT;
    pulse[0] = count;
    pulse[1] = start;
    for (i = 0; i < count; i++) {
        pulse[2 + 2 * i] = aac_getbits(5);
        pulse[3 + 2 * i] = aac_getbits(4);
    }
    return AAC_OK;
}

static u16 apply_pulse(u16 ch)
{
    u16 k = d_swb_long[pulse[1]], i, base = ch ? 2048 : 0;
    for (i = 0; i < pulse[0]; i++) {
        s16 amp = (s16)pulse[3 + 2 * i], v;
        k += pulse[2 + 2 * i];
        if (k >= 1024)
            return AAC_CORRUPT;
        v = (s16)d_spec[base + 2 * k + 1];  // quantized values fit in 16 bits
        v += apply_sign(amp, v <= 0);
        put32((u16)(unsigned)d_spec + base + 2 * k, v);
    }
    return AAC_OK;
}

static u16 parse_ics(u16 ch, u16 common_window, u16 allow_intensity)
{
    u16 global_gain = aac_getbits(8), pulse_present, s;
    if (!common_window) {
        s = parse_ics_info(ch);
        if (s != AAC_OK)
            return s;
    }
    s = parse_section_data(ch, allow_intensity);
    if (s != AAC_OK)
        return s;
    s = parse_scalefactors(ch, global_gain);
    if (s != AAC_OK)
        return s;
    pulse_present = aac_getbits(1);
    if (pulse_present) {
        s = parse_pulse(ch);
        if (s != AAC_OK)
            return s;
    }
    d_tns_count[ch] = 0;
    if (aac_getbits(1)) {
        s = parse_tns(ch);
        if (s != AAC_OK)
            return s;
    }
    if (aac_getbits(1))                 // gain control: AAC SSR only
        return AAC_UNSUPPORTED;
    if (overrun())
        return AAC_CORRUPT;
    s = parse_spectral_data(ch);
    if (s != AAC_OK)
        return s;
    if (pulse_present) {
        s = apply_pulse(ch);
        if (s != AAC_OK)
            return s;
    }
    if (overrun())
        return AAC_CORRUPT;
    return AAC_OK;
}

static u16 decode_sce(void)
{
    u16 s = parse_ics(0, 0, 0);
    if (s != AAC_OK)
        return s;
    aac_dsp_stages_sce();
    return AAC_OK;
}

static u16 decode_cpe(void)
{
    u16 common_window = aac_getbits(1), s, i;
    d_ms_mask_present = 0;
    if (common_window) {
        u16 g, gb, sfb, groups, max_sfb;
        s = parse_ics_info(0);
        if (s != AAC_OK)
            return s;
        for (i = 0; i < ICS_WORDS; i++)
            d_ics[ICS_WORDS + i] = d_ics[i];
        d_ms_mask_present = aac_getbits(2);
        if (d_ms_mask_present == 3)
            return AAC_CORRUPT;
        groups = d_ics[I_GROUPS];
        max_sfb = d_ics[I_MAX_SFB];
        for (g = 0, gb = 0; g < groups; g++, gb += NSFB)
            for (sfb = 0; sfb < max_sfb; sfb++) {
                u16 used = 0;
                if (d_ms_mask_present == 2)
                    used = 1;
                else if (d_ms_mask_present)
                    used = aac_getbits(1);
                d_ms_used[gb + sfb] = used;
            }
    }
    s = parse_ics(0, common_window, 0);
    if (s != AAC_OK)
        return s;
    s = parse_ics(1, common_window, common_window);
    if (s != AAC_OK)
        return s;
    aac_dsp_stages_cpe(common_window);
    return AAC_OK;
}

static void skip_bytes(u16 count)
{
    aac_bs_pos += count << 3;
}

static void skip_dse(void)
{
    u16 align_flag, count;
    aac_getbits(4);
    align_flag = aac_getbits(1);
    count = aac_getbits(8);
    if (count == 255)
        count += aac_getbits(8);
    if (align_flag)
        align();
    skip_bytes(count);
}

static void skip_fil(void)
{
    u16 count = aac_getbits(4), type = 0;
    if (count == 15)
        count += aac_getbits(8) - 1;
    if (count) {
        u16 save = aac_bs_pos;
        type = aac_getbits(4);
        aac_bs_pos = save;
    }
    if (type == 13 || type == 14)       // SBR data
        d_sbr_seen = 1;
    skip_bytes(count);
}

static void skip_pce(void)
{
    u16 front, side, back, lfe, assoc, cc;
    aac_getbits(10);
    front = aac_getbits(4);
    side = aac_getbits(4);
    back = aac_getbits(4);
    lfe = aac_getbits(2);
    assoc = aac_getbits(3);
    cc = aac_getbits(4);
    if (aac_getbits(1))
        aac_getbits(4);
    if (aac_getbits(1))
        aac_getbits(4);
    if (aac_getbits(1))
        aac_getbits(3);
    aac_bs_pos += (front + side + back) * 5 + lfe * 4 + assoc * 4 + cc * 5;
    align();
    skip_bytes(aac_getbits(8));
}

static u16 raw_block(void)
{
    u16 audio = 0;
    for (;;) {
        u16 id = aac_getbits(3), s = AAC_OK;
        if (overrun())
            return AAC_CORRUPT;
        if (id <= 1) {                  // SCE, CPE
            if (audio)
                return AAC_CORRUPT;
            if ((id == 0) != (d_channels == 1))
                return AAC_CORRUPT;
            aac_getbits(4);
            s = id == 0 ? decode_sce() : decode_cpe();
            audio = 1;
        } else if (id <= 3) {           // CCE, LFE
            return AAC_UNSUPPORTED;
        } else if (id == 4) {
            skip_dse();
        } else if (id == 5) {
            skip_pce();
        } else if (id == 6) {
            skip_fil();
        } else {
            align();
            if (overrun())
                return AAC_CORRUPT;
            if (audio)
                return AAC_OK;
            return AAC_CORRUPT;
        }
        if (s != AAC_OK)
            return s;
    }
}

void aac_dsp_reset(void)
{
    aac_stages_reset();
    d_configured = 0;
    d_synced = 0;
}

// One whole ADTS frame (sync checked by the caller's framing). Header fields
// as in decoder.c; the CRC words are skipped, not checked.
u16 aac_dsp_frame(u16 bytes)
{
    u16 protection_absent, object_type, sf, channel_config, frame_bytes, blocks, header_bytes, i, s;
    if (bytes < 7) return AAC_NEED_MORE_DATA;
    if (bytes > 2 * BS_WORDS) return AAC_UNSUPPORTED;
    if ((aac_bs_data[0] & 0xF6FF) != 0xF0FF) return AAC_RESYNC;
    aac_bs_swap((bytes + 1) >> 1);
    aac_bs_data[(bytes + 1) >> 1] = 0;
    aac_bs_data[((bytes + 1) >> 1) + 1] = 0;
    aac_bs_pos = 15;
    protection_absent = aac_getbits(1);
    object_type = aac_getbits(2) + 1;
    sf = aac_getbits(4);
    aac_getbits(1);
    channel_config = aac_getbits(3);
    aac_getbits(4);
    frame_bytes = aac_getbits(13);
    aac_getbits(11);
    blocks = aac_getbits(2) + 1;
    header_bytes = 7 + (protection_absent ? 0 : 2 * blocks);
    if (blocks != 1) return AAC_UNSUPPORTED;
    if (sf >= 13 || frame_bytes <= header_bytes || frame_bytes > bytes)
        return AAC_RESYNC;
    if (frame_bytes > 2 * BS_WORDS)     // LC needs at most 1536 bytes for two channels
        return AAC_UNSUPPORTED;
    if (object_type != 2 || sf > 11 || channel_config < 1 || channel_config > 2)
        return AAC_UNSUPPORTED;
    if (!d_configured || d_sf_index != sf || d_channels != channel_config) {
        // new stream parameters: overlap and window shapes start over
        aac_stages_clear();
        d_sf_index = sf;
        d_channels = channel_config;
        d_configured = 1;
        select_rate(sf);
    }
    aac_bs_bits = frame_bytes << 3;
    aac_bs_pos = header_bytes << 3;
    d_sbr_seen = 0;
    for (i = 0; i < blocks; i++) {
        s = raw_block();
        if (s != AAC_OK) {
            aac_stages_clear();
            d_synced = 0;
            return s;
        }
        if (!protection_absent && blocks > 1)
            aac_bs_pos += 16;
        if (overrun()) {
            aac_stages_clear();
            d_synced = 0;
            return AAC_CORRUPT;
        }
    }
    return AAC_OK;
}
