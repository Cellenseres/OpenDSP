// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include <string.h>
#include "opendsp.h"

#define ERR_NO_IMAGE MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_DSP, RD_NOT_FOUND)

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
    if (!opendspCheckImage(image, size))
        return ERR_NO_IMAGE;
    // ndspInit() then loads this image instead of dspfirm.cdc
    ndspUseComponent(image, size, 0xFF, 0xFF);
    return ndspInit();
}

void opendspExit(void)
{
    ndspExit();
}
