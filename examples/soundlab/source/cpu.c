// SPDX-License-Identifier: BSD-2-Clause
#include "cpu.h"

// Longer gaps mean another thread ran.
#define IDLE_GAP (SYSCLOCK_ARM11 / 100000)

static Thread thread;
static aptHookCookie hook;
static volatile bool running, paused;
static volatile u32 idle;

static void idle_main(void *argument)
{
    (void)argument;
    u64 last = svcGetSystemTick();
    while (running) {
        if (paused) {
            svcSleepThread(10000000);
            last = svcGetSystemTick();
            continue;
        }
        u64 now = svcGetSystemTick();
        if (now - last < IDLE_GAP)
            idle += (u32)(now - last);
        last = now;
    }
}

// No spinning during HOME or sleep.
static void on_apt(APT_HookType type, void *parameter)
{
    (void)parameter;
    if (type == APTHOOK_ONSUSPEND || type == APTHOOK_ONSLEEP)
        paused = true;
    else if (type == APTHOOK_ONRESTORE || type == APTHOOK_ONWAKEUP)
        paused = false;
}

void cpu_meter_start(void)
{
    if (thread)
        return;
    running = true;
    paused = false;
    // Lowest priority, the app core.
    thread = threadCreate(idle_main, NULL, 4096, 0x3F, -2, false);
    if (!thread) {
        running = false;
        return;
    }
    aptHook(&hook, on_apt, NULL);
}

void cpu_meter_stop(void)
{
    if (!thread)
        return;
    running = false;
    threadJoin(thread, U64_MAX);
    threadFree(thread);
    thread = NULL;
    aptUnhook(&hook);
}

bool cpu_meter_idle(u32 *ticks)
{
    *ticks = idle;
    return thread && !paused;
}
