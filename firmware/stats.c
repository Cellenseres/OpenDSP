// SPDX-License-Identifier: BSD-2-Clause
// Frame timing counters for opendspGetStats().
#include "stats.h"

#define TMR1_CFG     MMIO(0x030)
#define TMR1_START_L MMIO(0x034)
#define TMR1_START_H MMIO(0x036)
#define TMR1_COUNT_L MMIO(0x038)
#define TMR1_COUNT_H MMIO(0x03A)
#define TMR_FREE_RUN 0x0608     // free running, count enabled, restart

#define PLAUSIBLE    0x00800000UL   // about 12 frames; longer means a misread

stats_t stats;
static u32 frame_start;
static u16 started;
static u32 aac_pending, aac_previous;

void stats_charge_aac(u32 ticks) { aac_pending += ticks; }
u32 stats_aac_used(void) { return aac_pending; }

// counts down; reread when the high word moved
static u32 timer_now(void)
{
    u16 hi, lo;
    do {
        hi = TMR1_COUNT_H;
        lo = TMR1_COUNT_L;
    } while (hi != TMR1_COUNT_H);
    return ((u32)hi << 16) | lo;
}

void stats_init(void)
{
    u16 *p = (u16 *)&stats, k;
    for (k = 0; k < sizeof(stats) / sizeof(u16); k++)
        p[k] = 0;
    started = 0;
    TMR1_START_L = 0xFFFF;
    TMR1_START_H = 0xFFFF;
    TMR1_CFG = TMR_FREE_RUN;
}

void stats_frame_begin(u16 backlog)
{
    u32 now = timer_now();
    aac_previous = aac_pending;
    aac_pending = 0;
    stats.frames++;
    if (backlog) {
        stats.late++;
        if (backlog > stats.max_backlog)
            stats.max_backlog = backlog;
    }
    u32 period = aac_timer_delta(frame_start, now);
    if (started && period < PLAUSIBLE) {
        stats.period_last = period;
        if (!stats.period_min || period < stats.period_min)
            stats.period_min = period;
        if (period > stats.period_max)
            stats.period_max = period;
    }
    started = 1;
    frame_start = now;
}

void stats_frame_end(void)
{
    u32 spent = aac_timer_delta(frame_start, timer_now());
    stats.render_raw_last = spent;
    spent += aac_previous;
    if (spent >= PLAUSIBLE)
        return;
    stats.render_last = spent;
    if (spent > stats.render_max)
        stats.render_max = spent;
}

void stats_reset_peaks(void)
{
    stats.late = 0;
    stats.max_backlog = 0;
    stats.fifo_empty = 0;
    stats.render_max = 0;
    stats.period_min = 0;
    stats.period_max = 0;
}
