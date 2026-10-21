// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <opendsp.h>
#include <opendsp/audio.h>

#define MONITOR_HISTORY 96
typedef struct {
    opendspStats dsp;
    opendspAudioStats stream;
    opendspAudioInfo info;
    bool have_dsp, have_stream, held;
    unsigned backend;
    float arm_decode;
    float dsp_history[MONITOR_HISTORY], arm_history[MONITOR_HISTORY];
    unsigned head, count;
    u64 sampled_at;
} monitor_view;

void monitor_reset(void);
bool monitor_update(u64 now);
void monitor_hold(void);
void monitor_rebase(void);
const monitor_view *monitor_get(void);
const char *monitor_backend(void);
void monitor_draw_top(void);
