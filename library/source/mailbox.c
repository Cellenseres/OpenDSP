// SPDX-License-Identifier: BSD-2-Clause
// Extension mailbox in DSP memory.
#include <string.h>
#include "opendsp.h"
#include "internal.h"
#include "extension.h"

#define WAIT_STEP_NS 500000LL
#define WAIT_STEPS 400 // 200 ms

static LightLock lock;
static bool lock_ready;

void opendsp_mailbox_setup(void)
{
    if (!lock_ready) {
        LightLock_Init(&lock);
        lock_ready = true;
    }
}

static vu16 *find_mailbox(void)
{
    u32 address;
    vu16 *box;
    if (R_FAILED(DSP_ConvertProcessAddressFromDspDram(EXT_BASE, &address)))
        return NULL;
    box = (vu16 *)address;
    if (box[EXT_MAGIC0] != EXT_MAGIC0_VALUE || box[EXT_MAGIC1] != EXT_MAGIC1_VALUE ||
        box[EXT_PROTOCOL] != EXT_PROTOCOL_VERSION)
        return NULL;
    return box;
}

static Result status_to_result(u16 status)
{
    switch (status) {
    case EXT_OK:
        return 0;
    case EXT_ERR_ARGUMENT:
        return OPENDSP_ERR_ARGUMENT;
    case EXT_ERR_ADDRESS:
        return OPENDSP_ERR_ADDRESS;
    default:
        return OPENDSP_ERR_UNSUPPORTED;
    }
}

Result opendsp_request(u16 command, const u16 *args, u16 arg_count, u16 *reply, u16 reply_count)
{
    vu16 *box;
    u16 sequence, i;
    Result rc;
    if (arg_count > EXT_ARG_WORDS || reply_count > EXT_REPLY_WORDS)
        return OPENDSP_ERR_ARGUMENT;
    if (!opendsp_running)
        return OPENDSP_ERR_NOT_RUNNING;
    LightLock_Lock(&lock);
    box = find_mailbox();
    if (!box) {
        LightLock_Unlock(&lock);
        return OPENDSP_ERR_NOT_OPENDSP;
    }
    for (i = 0; i < arg_count; i++)
        box[EXT_ARGS + i] = args[i];
    box[EXT_COMMAND] = command;
    sequence = (u16)(box[EXT_ACK] + 1);
    __dsb();
    box[EXT_SEQUENCE] = sequence;
    __dsb();
    rc = OPENDSP_ERR_TIMEOUT;
    for (i = 0; i < WAIT_STEPS; i++) {
        if (box[EXT_ACK] == sequence) {
            rc = status_to_result(box[EXT_STATUS]);
            for (u16 k = 0; k < reply_count; k++)
                reply[k] = box[EXT_REPLY + k];
            break;
        }
        svcSleepThread(WAIT_STEP_NS);
    }
    LightLock_Unlock(&lock);
    return rc;
}

Result opendspGetCapabilities(u32 *caps)
{
    vu16 *box;
    u32 firmware = 0;
    if (!caps)
        return OPENDSP_ERR_ARGUMENT;
    if (!opendsp_running) {
        *caps = 0;
        return 0;
    }
    LightLock_Lock(&lock);
    box = find_mailbox();
    if (box)
        firmware = (((u32)box[EXT_CAPS_HI] << 16) | box[EXT_CAPS_LO]) & ~(u32)EXT_CAP_MAILBOX;
    LightLock_Unlock(&lock);
    // the firmware bits match OPENDSP_CAP_*
    *caps = firmware | (box ? OPENDSP_CAP_FIRMWARE : 0);
    return 0;
}

Result opendspGetFirmwareVersion(u16 *version)
{
    vu16 *box;
    if (!version)
        return OPENDSP_ERR_ARGUMENT;
    *version = 0;
    if (!opendsp_running)
        return OPENDSP_ERR_NOT_RUNNING;
    LightLock_Lock(&lock);
    box = find_mailbox();
    *version = box ? box[EXT_VERSION] : 0;
    LightLock_Unlock(&lock);
    return box ? 0 : OPENDSP_ERR_NOT_OPENDSP;
}
