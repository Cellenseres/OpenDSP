// SPDX-License-Identifier: BSD-2-Clause
// Per-source configuration, rendering and status.
#pragma once
#include "voice.h"

#define NUM_SOURCES 24

void sources_init(void);
void sources_render(u16 read_bank, u16 write_bank, vu16 *out);
s32 float_to_fixed(u16 lo, u16 hi, u16 frac_bits);
