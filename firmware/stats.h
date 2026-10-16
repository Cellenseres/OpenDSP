// SPDX-License-Identifier: BSD-2-Clause
// Frame timing counters for opendspGetStats().
#pragma once
#include "hw.h"

typedef struct {
    u32 frames;
    u16 late;               // frames rendered with another one already due
    u16 max_backlog;
    u16 fifo_empty;         // output FIFO found empty when refilled
    u16 pad;
    u32 render_last, render_max, period_last;   // timer ticks
    u32 period_min, period_max;                 // differ if the output drains unevenly
} stats_t;

extern stats_t stats;

void stats_init(void);
void stats_frame_begin(u16 backlog);
void stats_frame_end(void);
void stats_reset_peaks(void);
