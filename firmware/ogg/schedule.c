// SPDX-License-Identifier: BSD-2-Clause
#include "schedule.h"
#include "hw.h"

static u32 slice_started;
static u16 checkpoints, running;
u32 aac_timer_now(void);
u32 aac_timer_elapsed(u32 start);
void aac_yield(void);
void aac_task_step(void);

void ogg_decode_step(void)
{
    running = 1;
    aac_task_step();
    running = 0;
}

void ogg_decode_begin(void)
{
    checkpoints = 0;
    slice_started = aac_timer_now();
}

void ogg_decode_yield(void)
{
    aac_yield();
    slice_started = aac_timer_now();
}

void tremor_decode_yield(void)
{
    if (running && (++checkpoints & 7) == 0 && aac_timer_elapsed(slice_started) >= 65536UL)
        ogg_decode_yield();
}
