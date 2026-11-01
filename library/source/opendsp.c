// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <string.h>
#include "opendsp.h"
#include "internal.h"
#include "codec_events.h"
#include "extension.h"
#include "sleep.h"

bool opendsp_running;
void opendsp_audio_before_exit(void) __attribute__((weak));
void opendsp_audio_after_exit(bool unloaded) __attribute__((weak));
void opendsp_audio_wakeup(bool restored) __attribute__((weak));
static dspHookCookie power_cookie;
static bool power_hooked, context_saved;
static void *sleep_memory;

bool opendsp_sleep_preserves_context(void) { return context_saved; }

static void power_after_ndsp(DSP_HookType type)
{
    if (type != DSPHOOK_ONWAKEUP) return;
    opendsp_codec_events_wakeup();
    u32 address = 0;
    bool restored = context_saved && R_SUCCEEDED(DSP_ConvertProcessAddressFromDspDram(EXT_BASE, &address)) &&
                    ((vu16 *)address)[SLEEP_RESTORED] == 1;
    opendsp_aac_wakeup(restored);
    if (opendsp_audio_wakeup) opendsp_audio_wakeup(restored);
}

bool opendspCheckImage(const void *image, u32 size)
{
    const u8 *b = image;
    u32 declared;
    if (!image || size < 0x300 || memcmp(b + 0x100, "DSP1", 4) != 0)
        return false;
    memcpy(&declared, b + 0x104, 4);
    return declared <= size;
}

Result opendspInitFromMemory(const void *image, u32 size)
{
    Result rc;
    if (!opendspCheckImage(image, size))
        return OPENDSP_ERR_NO_IMAGE;
    opendsp_mailbox_setup();
    if (!power_hooked) {
        dspHook(&power_cookie, power_after_ndsp);
        power_hooked = true;
    }
    // ndspInit() then loads this image instead of dspfirm.cdc
    ndspUseComponent(image, size, 0xFF, 0xFF);
    rc = ndspInit();
    if (R_SUCCEEDED(rc)) {
        opendsp_running = true;
        opendsp_aac_power_order();
        u32 caps = 0;
        opendspGetCapabilities(&caps);
        if (caps & OPENDSP_CAP_SLEEP) {
            if (!sleep_memory) sleep_memory = linearMemAlign(SLEEP_EXT_BYTES, 128);
            rc = sleep_memory ? 0 : OPENDSP_ERR_NO_MEMORY;
            if (sleep_memory) {
                memset(sleep_memory, 0, SLEEP_EXT_BYTES);
                DSP_FlushDataCache(sleep_memory, SLEEP_EXT_BYTES);
                DSP_InvalidateDataCache(sleep_memory, SLEEP_EXT_BYTES);
                u32 physical = osConvertVirtToPhys(sleep_memory);
                u16 args[4] = {physical >> 16, physical, SLEEP_EXT_BYTES >> 16, (u16)SLEEP_EXT_BYTES};
                rc = opendsp_request(EXT_CMD_SLEEP_SET, args, 4, NULL, 0);
                context_saved = R_SUCCEEDED(rc);
            }
            if (R_FAILED(rc)) opendspExit();
        }
    }
    if (R_FAILED(rc) && power_hooked) {
        dspUnhook(&power_cookie);
        power_hooked = false;
    }
    return rc;
}

void opendspExit(void)
{
    if (opendsp_audio_before_exit)
        opendsp_audio_before_exit();
    opendsp_codec_events_stop();
    ndspExit();
    opendsp_running = false;
    context_saved = false;
    if (power_hooked) { dspUnhook(&power_cookie); power_hooked = false; }
    if (sleep_memory && !dspIsComponentLoaded()) { linearFree(sleep_memory); sleep_memory = NULL; }
    opendsp_aac_release();
    opendsp_effects_release();
    if (opendsp_audio_after_exit)
        opendsp_audio_after_exit(!dspIsComponentLoaded());
}
