// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <opendsp.h>
#include <opendsp/audio.h>

#define MONITOR_HISTORY 96
enum {
    SERIES_DSP_TOTAL,
    SERIES_DSP_DECODE,
    SERIES_DSP_MIX, // voices, mixing and effects
    SERIES_CPU_TOTAL, // core 0
    SERIES_CPU_DECODE, // ARM11 decoders, without SD reads
    SERIES_SD_READ, // waiting, not CPU
    MONITOR_SERIES
};
typedef struct {
    opendspStats dsp;
    opendspLoad load;
    opendspAudioStats stream;
    opendspAudioInfo info;
    opendspAudioDecoderStats decoder; // DSP decoders only
    bool have_dsp, have_load, have_stream, have_decoder, held;
    bool loop_cached; // playing the loop from memory
    unsigned backend;
    float value[MONITOR_SERIES]; // time share, -1 if unknown
    float history[MONITOR_SERIES][MONITOR_HISTORY];
    unsigned head, count;
    u64 sampled_at;
} monitor_view;

void monitor_reset(void);
bool monitor_update(u64 now);
void monitor_hold(void);
void monitor_rebase(void);
const monitor_view *monitor_get(void);
float monitor_peak(unsigned series); // -1 if unknown
const char *monitor_backend(void);
void monitor_draw_top(void);
