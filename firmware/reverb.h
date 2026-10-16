// SPDX-License-Identifier: BSD-2-Clause
// Reverb: input diffusion and a four line feedback delay network.
#pragma once
#include "hw.h"

#define REVERB_LINES 4

typedef struct {
    u16 enable;
    u16 length[REVERB_LINES];     // samples
    s16 gain[REVERB_LINES];       // feedback, Q15
    s16 damping, diffusion;       // Q15
    s16 out_same, out_cross;      // Q15
    s16 input_gain;               // Q15, on (FL + FR + RL + RR) / 2
} reverb_params_t;

void reverb_init(void);
u16 reverb_set(u16 bus, const reverb_params_t *params);
u16 reverb_enabled(u16 bus);

// quad: front L, front R, rear L, rear R; stride 2. Output wet in front.
void reverb_run(u16 bus, s32 **quad);
