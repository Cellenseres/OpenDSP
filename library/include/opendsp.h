// SPDX-License-Identifier: BSD-2-Clause
// OpenDSP: open-source DSP firmware for libctru's ndsp.
#pragma once
#include <3ds/types.h>
#include <3ds/result.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENDSP_VERSION_MAJOR 1
#define OPENDSP_VERSION_MINOR 0
#define OPENDSP_VERSION_PATCH 0

// Replaces ndspInit().
Result opendspInit(void);

// Own image; keep it valid until opendspExit().
Result opendspInitFromMemory(const void *image, u32 size);

// Stop ndsp. Use instead of ndspExit().
void opendspExit(void);

// Checks for a DSP1 header.
bool opendspCheckImage(const void *image, u32 size);

#ifdef __cplusplus
}
#endif
