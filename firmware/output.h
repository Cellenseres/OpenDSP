// SPDX-License-Identifier: BSD-2-Clause
// Audio output and frame pacing.
#pragma once
#include "hw.h"

#define OUTPUT_BUFFERS 0x7000   // 2 x 320 words, below MMIO

extern volatile u16 frames_due;
extern volatile u16 render_buffer;
extern u16 output_running;

vu16 *output_buffer(u16 index);
void output_start(void);
// Tops the FIFO up. Also called before every kernel that runs with interrupts
// off: the BTDMP IRQ only fires once the FIFO is empty, which leaves one sample
// period to refill it; a full FIFO gives eight.
void output_poll(void);
void output_stop(void);
