// SPDX-License-Identifier: BSD-2-Clause
// OpenDSP: open-source DSP firmware for libctru's ndsp.
#pragma once
#include <3ds/types.h>
#include <3ds/result.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OPENDSP_VERSION_MAJOR 1
#define OPENDSP_VERSION_MINOR 1
#define OPENDSP_VERSION_PATCH 0

#define OPENDSP_ERR_NO_IMAGE MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_DSP, RD_NOT_FOUND)
#define OPENDSP_ERR_NOT_RUNNING MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_DSP, RD_NOT_INITIALIZED)
#define OPENDSP_ERR_NOT_OPENDSP MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, RD_NOT_FOUND)
#define OPENDSP_ERR_UNSUPPORTED MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, RD_NOT_IMPLEMENTED)
#define OPENDSP_ERR_ARGUMENT MAKERESULT(RL_PERMANENT, RS_INVALIDARG, RM_DSP, RD_OUT_OF_RANGE)
#define OPENDSP_ERR_ADDRESS MAKERESULT(RL_PERMANENT, RS_INVALIDARG, RM_DSP, RD_INVALID_ADDRESS)
#define OPENDSP_ERR_NO_MEMORY MAKERESULT(RL_PERMANENT, RS_OUTOFRESOURCE, RM_DSP, RD_OUT_OF_MEMORY)
#define OPENDSP_ERR_TIMEOUT MAKERESULT(RL_TEMPORARY, RS_NOTFOUND, RM_DSP, RD_TIMEOUT)

// opendspGetCapabilities() bits
#define OPENDSP_CAP_FIRMWARE (1u << 0) // OpenDSP firmware is running
#define OPENDSP_CAP_DELAY (1u << 1)
#define OPENDSP_CAP_REVERB (1u << 2)

// Replaces ndspInit().
Result opendspInit(void);

// Own image; keep it valid until opendspExit().
Result opendspInitFromMemory(const void *image, u32 size);

// Stop ndsp. Use instead of ndspExit().
void opendspExit(void);

// Checks for a DSP1 header.
bool opendspCheckImage(const void *image, u32 size);

// Features of the running firmware.
Result opendspGetCapabilities(u32 *caps);

// Firmware version, major << 8 | minor.
Result opendspGetFirmwareVersion(u16 *version);

#ifdef __cplusplus
}
#endif
