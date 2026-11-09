// SPDX-License-Identifier: BSD-2-Clause
#include "hw.h"
void ogg_opus_pause(void);

#ifndef OPUS_SLICE_CYCLES
#define OPUS_SLICE_CYCLES 65536UL
#endif
static u32 slice_started;
// word below OGG_TASK_GUARD, shared with opus_poll.s
#define ogg_opus_checkpoints (*(vu16 *)0x45ff)
u32 ogg_opus_clock(void);
u32 ogg_opus_elapsed(u32 started);
void output_kick(void);

void ogg_opus_resume(void)
{
    ogg_opus_checkpoints = 0;
    slice_started = ogg_opus_clock();
}

// fast path lives in opus_poll.s
void ogg_opus_poll_slow(void)
{
    if (!(BTDMP0_TX_STAT & BTDMP_TX_FULL))
        output_kick();
#ifdef POLL_GAP
    {
        u32 gap = ogg_opus_elapsed(slice_started);
        vu16 *slot = (vu16 *)0x45fc;
        if ((gap >> 16) > slot[0] || ((gap >> 16) == slot[0] && (u16)gap > slot[1])) {
            slot[0] = (u16)(gap >> 16);
            slot[1] = (u16)gap;
        }
    }
#endif
    if ((ogg_opus_checkpoints & 7) == 0 && ogg_opus_elapsed(slice_started) >= OPUS_SLICE_CYCLES)
        ogg_opus_pause();
}
