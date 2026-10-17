// SPDX-License-Identifier: BSD-2-Clause
// AAC-LC decoder on the DSP: state shared by the parser and the stages.
// Results match library/source/aac (the ARM11 decoder) bit for bit.
#pragma once
#include "hw.h"
void aac_yield(void);

enum {
    AAC_OK = 0, AAC_NEED_MORE_DATA, AAC_RESYNC, AAC_CORRUPT, AAC_UNSUPPORTED,
    AAC_OUTPUT_TOO_SMALL, AAC_INVALID_ARGUMENT,
};

#define EIGHT_SHORT 2
#define NSFB 51
#define BANDS (8 * NSFB)                // [group * NSFB + sfb]
#define BS_WORDS 1024                   // frame buffer: 2048 bytes

// per channel ICS info, at d_ics[ch * ICS_WORDS]
enum { I_SEQ, I_SHAPE, I_MAX_SFB, I_NUM_SWB, I_WINDOWS, I_GROUPS, I_GLEN, ICS_WORDS = I_GLEN + 8 };
// TNS filter, at d_tns[(ch * 8 + f) * TNS_WORDS]
enum { T_WINDOW, T_BOTTOM, T_TOP, T_ORDER, T_DIRECTION, T_COEF, TNS_WORDS = T_COEF + 12 };

extern u16 aac_bs_data[BS_WORDS + 2];
extern u16 aac_bs_bits;                 // frame length in bits
// bit reader state, fixed so the asm can reach it: position, buffer address
#define aac_bs_pos (*(u16 *)0x0FF0)
#define AAC_BS_BASE (*(u16 *)0x0FF1)

extern u16 d_ics[2 * ICS_WORDS];
extern u16 d_band_type[2 * BANDS];
extern s16 d_scalefactor[2 * BANDS];
extern u16 d_ms_used[BANDS];
extern u16 d_tns[2 * 8 * TNS_WORDS];
extern u16 d_tns_count[2];
extern u16 d_spec[2 * 2048];            // s32, high word first
extern u16 d_swb_long[52], d_swb_short[16];
extern u16 d_ms_mask_present, d_sf_index, d_channels, d_configured, d_synced, d_sbr_seen;
extern u16 d_prev_shape[2];
extern u16 d_overlap[2], d_pcm;
void aac_stages_clear(void);
void aac_stages_reset(void);
void aac_stage_dequant(u16 ch);
void aac_stage_noise(u16 stereo);
void aac_stage_ms(void);
void aac_stage_intensity(void);
void aac_stage_tns(u16 ch);
void aac_stage_filterbank(u16 ch);

u16 aac_getbits(u16 n);
u16 aac_huff(u16 table);
u16 aac_decode_band(const u16 *job);    // type, table, dst, count
void aac_bs_swap(u16 words);

void aac_dsp_init(void);
void aac_dsp_reset(void);
// One ADTS frame of 'bytes' bytes in aac_bs_data (little-endian words, as
// DMAed). Returns an AAC_* status; PCM per channel in the filterbank output.
u16 aac_dsp_frame(u16 bytes);
