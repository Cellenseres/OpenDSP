// SPDX-License-Identifier: BSD-2-Clause
// AAC filterbank on the DSP: IMDCT, windows, overlap-add, PCM.
#pragma once
#include "hw.h"

// word addresses of the tables and buffers the filterbank uses
typedef struct {
    u16 stage_tw;               // FFT twiddles per stage (Y)
    u16 imdct_long, imdct_short;    // pre/post twiddles (Y)
    u16 bitrev;                 // 4 bitrev9(2i), i < 256
    u16 sine_long, kbd_long, sine_short, kbd_short;
    u16 z;                      // 2048 words: complex s32
    u16 t;                      // 4096 words: time samples s32
    u16 tmp;                    // 512 words
} aac_fb_t;

extern aac_fb_t aac_fb;

// Called before every interrupt-free kernel span.
void aac_yield(void);

// One channel: x = spectrum (1024 s32, overwritten), overlap = 1024 s32,
// pcm = 1024 s16. mode = window sequence | shape << 2 | previous shape << 3.
void aac_fb_channel(u16 x, u16 overlap, u16 pcm, u16 mode);
