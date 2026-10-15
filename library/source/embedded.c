// SPDX-License-Identifier: BSD-2-Clause
#include <3ds.h>
#include "opendsp.h"

extern const u8 opendsp_firmware[];     // firmware.s
extern const u32 opendsp_firmware_size;

Result opendspInit(void)
{
    return opendspInitFromMemory(opendsp_firmware, opendsp_firmware_size);
}
