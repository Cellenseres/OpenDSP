/* SPDX-License-Identifier: BSD-2-Clause */
/* Pipe-4 notifications leave ndsp callbacks and pipe 2 intact. */
#include "codec_events.h"
#include "mp3_protocol.h"
static Handle irq;
static Thread dispatcher;
static LightEvent completed, mailbox;
static bool running;
static dspHookCookie hook;
void opendsp_audio_codec_notify(void) __attribute__((weak));

bool opendsp_codec_events_active(void)
{
    return __atomic_load_n(&running, __ATOMIC_ACQUIRE);
}
static void signal_waiters(void)
{
    LightEvent_Signal(&completed);
    LightEvent_Signal(&mailbox);
    if (opendsp_audio_codec_notify)
        opendsp_audio_codec_notify();
}
static void dispatch(void *unused)
{
    (void)unused;
    while (opendsp_codec_events_active()) {
        Result r = svcWaitSynchronization(irq, U64_MAX);
        if (R_FAILED(r))
            break;
        signal_waiters();
    }
}
static void power(DSP_HookType type)
{
    if (type == DSPHOOK_ONWAKEUP && opendsp_codec_events_active())
        DSP_RegisterInterruptEvents(irq, DSP_INTERRUPT_PIPE, MP3_EVENT_PIPE);
    signal_waiters();
}
Result opendsp_codec_events_start(void)
{
    if (opendsp_codec_events_active())
        return 0;
    Result r = svcCreateEvent(&irq, RESET_ONESHOT);
    if (R_FAILED(r))
        return r;
    r = DSP_RegisterInterruptEvents(irq, DSP_INTERRUPT_PIPE, MP3_EVENT_PIPE);
    if (R_FAILED(r)) {
        svcCloseHandle(irq);
        irq = 0;
        return r;
    }
    LightEvent_Init(&completed, RESET_ONESHOT);
    LightEvent_Init(&mailbox, RESET_ONESHOT);
    __atomic_store_n(&running, true, __ATOMIC_RELEASE);
    s32 priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    dispatcher = threadCreate(dispatch, NULL, 4096, priority, -2, false);
    if (!dispatcher) {
        __atomic_store_n(&running, false, __ATOMIC_RELEASE);
        DSP_RegisterInterruptEvents(0, DSP_INTERRUPT_PIPE, MP3_EVENT_PIPE);
        svcCloseHandle(irq);
        irq = 0;
        return MAKERESULT(RL_TEMPORARY, RS_OUTOFRESOURCE, RM_DSP, 0x2c1);
    }
    dspHook(&hook, power);
    return 0;
}
void opendsp_codec_events_stop(void)
{
    if (!opendsp_codec_events_active())
        return;
    DSP_RegisterInterruptEvents(0, DSP_INTERRUPT_PIPE, MP3_EVENT_PIPE);
    __atomic_store_n(&running, false, __ATOMIC_RELEASE);
    svcSignalEvent(irq);
    threadJoin(dispatcher, U64_MAX);
    threadFree(dispatcher);
    dispatcher = NULL;
    dspUnhook(&hook);
    svcCloseHandle(irq);
    irq = 0;
    signal_waiters();
}
void opendsp_codec_wait(s64 ns)
{
    LightEvent_WaitTimeout(&completed, ns);
}
void opendsp_mailbox_wait(s64 ns)
{
    LightEvent_WaitTimeout(&mailbox, ns);
}
