// SPDX-License-Identifier: BSD-2-Clause
// From tools/gen_aac_tables.py
#pragma once
#include <stdint.h>

extern const uint32_t aac_sample_rates[12];
extern const uint16_t aac_swb_long_96[42];
extern const uint16_t aac_swb_long_64[48];
extern const uint16_t aac_swb_long_48[50];
extern const uint16_t aac_swb_long_32[52];
extern const uint16_t aac_swb_long_24[48];
extern const uint16_t aac_swb_long_16[44];
extern const uint16_t aac_swb_long_8[41];
extern const uint16_t aac_swb_short_96[13];
extern const uint16_t aac_swb_short_48[15];
extern const uint16_t aac_swb_short_24[16];
extern const uint16_t aac_swb_short_16[16];
extern const uint16_t aac_swb_short_8[16];
extern const uint16_t *const aac_swb_long[12];
extern const uint16_t *const aac_swb_short[12];
extern const uint8_t aac_num_swb_long[12];
extern const uint8_t aac_num_swb_short[12];
extern const uint8_t aac_tns_max_bands_long[12];
extern const uint8_t aac_tns_max_bands_short[12];
extern const uint16_t aac_huff_sf[382];
extern const uint16_t aac_huff_1[304];
extern const uint16_t aac_huff_2[270];
extern const uint16_t aac_huff_3[332];
extern const uint16_t aac_huff_4[300];
extern const uint16_t aac_huff_5[316];
extern const uint16_t aac_huff_6[294];
extern const uint16_t aac_huff_7[306];
extern const uint16_t aac_huff_8[268];
extern const uint16_t aac_huff_9[434];
extern const uint16_t aac_huff_10[366];
extern const uint16_t aac_huff_11[462];
extern const uint16_t *const aac_huff_tables[12];
extern const uint8_t aac_huff_root_bits[12];
extern const int16_t aac_sine_long[1024];
extern const int16_t aac_sine_short[128];
extern const int16_t aac_kbd_long[1024];
extern const int16_t aac_kbd_short[128];
extern const int16_t aac_fft_twiddle[512];
extern const int16_t aac_imdct_twiddle_long[1024];
extern const int16_t aac_imdct_twiddle_short[128];
extern const uint16_t aac_bitrev512[512];
extern const int32_t aac_pow43[1025];
extern const int16_t aac_pow2_quarter[4];
extern const int16_t aac_pow2_neg_quarter[4];
extern const uint16_t aac_inv_sqrt[49];
extern const int16_t aac_tns_coef_3bit[8];
extern const int16_t aac_tns_coef_4bit[16];
