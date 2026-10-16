// SPDX-License-Identifier: BSD-2-Clause
// Surround output mode: rear channels folded into stereo.
#pragma once
#include "hw.h"

void surround_init(void);
// returns 1 when the rear ratio changed
u16 surround_parse(u16 dirty_hi, vu16 *config);
u16 surround_rear_ratio(void);

// interleaved L R accumulators (Q14), 160 samples. mode 1: main += ratio * rear,
// mode 2: main += rear + post (no speaker virtualisation yet).
void surround_run(s32 *main, const s32 *rear, const s32 *post, u16 mode);
