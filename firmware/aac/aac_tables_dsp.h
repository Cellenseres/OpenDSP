// SPDX-License-Identifier: BSD-2-Clause
// From gen_dsp_tables.py.
#pragma once
#include "hw.h"

extern const u16 daac_swb_long_96[42];
extern const u16 daac_swb_long_64[48];
extern const u16 daac_swb_long_48[50];
extern const u16 daac_swb_long_32[52];
extern const u16 daac_swb_long_24[48];
extern const u16 daac_swb_long_16[44];
extern const u16 daac_swb_long_8[41];
extern const u16 daac_swb_short_96[13];
extern const u16 daac_swb_short_48[15];
extern const u16 daac_swb_short_24[16];
extern const u16 daac_swb_short_16[16];
extern const u16 daac_swb_short_8[16];
extern const u16 daac_num_swb_long[12];
extern const u16 daac_num_swb_short[12];
extern const u16 daac_tns_max_bands_long[12];
extern const u16 daac_tns_max_bands_short[12];
extern const u16 daac_huff_sf[382];
extern const u16 daac_huff_1[304];
extern const u16 daac_huff_2[270];
extern const u16 daac_huff_3[332];
extern const u16 daac_huff_4[300];
extern const u16 daac_huff_5[316];
extern const u16 daac_huff_6[294];
extern const u16 daac_huff_7[306];
extern const u16 daac_huff_8[268];
extern const u16 daac_huff_9[434];
extern const u16 daac_huff_10[366];
extern const u16 daac_huff_11[462];
extern const u16 daac_huff_root_bits[12];
extern const s16 daac_sine_long[1024];
extern const s16 daac_sine_short[128];
extern const s16 daac_kbd_long[1024];
extern const s16 daac_kbd_short[128];
extern const s16 daac_imdct_twiddle_long[1024];
extern const s16 daac_imdct_twiddle_short[128];
extern const u16 daac_pow43[2050];   // s32 pairs, high first
extern const s16 daac_pow2_quarter[4];
extern const s16 daac_pow2_neg_quarter[4];
extern const u16 daac_inv_sqrt[49];
extern const s16 daac_tns_coef_3bit[8];
extern const s16 daac_tns_coef_4bit[16];
extern const s16 d_aac_fft_stage_twiddle[1020];
extern const u16 d_aac_head_bitrev[256];
// aac_swb_long: aac_swb_long_96, aac_swb_long_96, aac_swb_long_64, aac_swb_long_48, aac_swb_long_48, aac_swb_long_32, aac_swb_long_24, aac_swb_long_24, aac_swb_long_16, aac_swb_long_16, aac_swb_long_16, aac_swb_long_8
// aac_swb_short: aac_swb_short_96, aac_swb_short_96, aac_swb_short_96, aac_swb_short_48, aac_swb_short_48, aac_swb_short_48, aac_swb_short_24, aac_swb_short_24, aac_swb_short_16, aac_swb_short_16, aac_swb_short_16, aac_swb_short_8
// aac_huff_tables: aac_huff_sf, aac_huff_1, aac_huff_2, aac_huff_3, aac_huff_4, aac_huff_5, aac_huff_6, aac_huff_7, aac_huff_8, aac_huff_9, aac_huff_10, aac_huff_11
