// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <string.h>
#include "opendsp.h"
#include "internal.h"

bool opendsp_running;

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
    // ndspInit() then loads this image instead of dspfirm.cdc
    ndspUseComponent(image, size, 0xFF, 0xFF);
    rc = ndspInit();
    if (R_SUCCEEDED(rc))
        opendsp_running = true;
    return rc;
}

void opendspExit(void)
{
    ndspExit();
    opendsp_running = false;
    opendsp_effects_release();
}
