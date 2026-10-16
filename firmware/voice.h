// SPDX-License-Identifier: BSD-2-Clause
// Voice engine: buffer queue, sample fetch, resampling.
#pragma once
#include "hw.h"

// second loop operand must be in Y
#define YMEM     __attribute__((section(".ydata")))
#define YMEM_RO  __attribute__((section(".yrodata")))
// not loaded at boot: whoever owns it clears it
#define YMEM_BSS __attribute__((section(".ybss")))

#ifdef __teak__
void clear_words(void *dst, u16 words);
#endif

enum { FMT_PCM8 = 0, FMT_PCM16 = 1, FMT_ADPCM = 2 };
enum { INTERP_POLYPHASE = 0, INTERP_LINEAR = 1, INTERP_NONE = 2 };

#define QUEUE_SIZE    5
#define MAX_STEP      0x40000UL                  // rate 4.0
#define WINDOW_FRAMES (FRAME_SAMPLES * 4 + 4)

// no char fields: teak-llvm layout limit
typedef struct {
    u32 address, length, start;
    u16 id;
    u16 looping, played;
    u16 adpcm_context;
    s16 yn1, yn2;
} wavebuf_t;

typedef struct {
    u16 enabled, format, stereo, interp;
    u32 step;                    // Q16
    wavebuf_t queue[QUEUE_SIZE];
    u16 queued;
    u16 has_current;
    wavebuf_t current;
    u32 position;
    u16 frac;
    u16 primed;
    u16 head_valid;              // real samples in x0, x1
    s16 x0[2], x1[2];
    s16 h1[2], h2[2];
    u16 x0_id, x1_id;
    u32 x0_position, x1_position;
    u16 last_valid_id;
    u16 current_id, last_id;
    u16 id_changed;
    u32 status_position;
    s16 coefs[16];
    s16 yn1, yn2;
    u32 adpcm_frame;
    u16 adpcm_nibble;
    u16 coef_slot, coef_version, slot_version;
} voice_t;

// pointer fields at even offsets, hence the pads
typedef struct { u32 address; u16 count, format, stereo, pad0; s16 *dst; u16 pad1; } fetch_job_t;
typedef struct {
    u32 step; const s16 *window; u16 pad0; s16 *out; u16 pad1;
    u16 stereo, interp, frac, count; const s16 *table; u16 pad2;
} resample_job_t;

void fetch_samples(const fetch_job_t *job);
void resample(const resample_job_t *job);
s32 mul16(s16 a, s16 b);

void voice_reset(voice_t *v);
int voice_queue(voice_t *v, const wavebuf_t *buf);
int voice_render(voice_t *v, s16 *out);
