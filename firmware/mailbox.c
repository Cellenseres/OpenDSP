// SPDX-License-Identifier: BSD-2-Clause
// Extension mailbox: capabilities and effect commands.
#include "mailbox.h"
#include "extension.h"
#include "mixer.h"
#include "source.h"
#include "reverb.h"
#include "stats.h"

#define BOX ((vu16 *)EXT_BASE)
#define FIRMWARE_VERSION 0x0101
#define CAPABILITIES (EXT_CAP_MAILBOX | EXT_CAP_DELAY | EXT_CAP_REVERB)

void mailbox_init(void)
{
    u16 i;
    for (i = 0; i < EXT_WORDS; i++)
        BOX[i] = 0;
    BOX[EXT_PROTOCOL] = EXT_PROTOCOL_VERSION;
    BOX[EXT_VERSION] = FIRMWARE_VERSION;
    BOX[EXT_CAPS_LO] = (u16)CAPABILITIES;
    BOX[EXT_CAPS_HI] = 0;
    BOX[EXT_MAGIC1] = EXT_MAGIC1_VALUE;
    BOX[EXT_MAGIC0] = EXT_MAGIC0_VALUE; // valid from here on
}

static u32 arg_u32(vu16 *args, u16 index)
{
    return ((u32)args[index] << 16) | args[index + 1];
}

static delay_params_t delay_params;

static u16 set_delay(vu16 *args)
{
    u16 bus = args[0], status;
    if (bus > 1)
        return EXT_ERR_ARGUMENT;
    delay_params.enable = args[1];
    delay_params.frames = args[2];
    delay_params.channels = 4;
    delay_params.g = (s16)args[3];
    delay_params.a = (s16)args[4];
    delay_params.b = (s16)args[5];
    delay_params.address = arg_u32(args, 6);
    delay_params.zeroed = args[8];
    status = mixer_set_delay(bus, &delay_params);
    sources_routing_changed();
    return status;
}

static reverb_params_t reverb_params;

static u16 set_reverb(vu16 *args)
{
    u16 bus = args[0], k, status;
    if (bus > 1)
        return EXT_ERR_ARGUMENT;
    reverb_params.enable = args[1];
    for (k = 0; k < REVERB_LINES; k++) {
        reverb_params.length[k] = args[2 + k];
        reverb_params.gain[k] = (s16)args[6 + k];
    }
    reverb_params.damping = (s16)args[10];
    reverb_params.diffusion = (s16)args[11];
    reverb_params.out_same = (s16)args[12];
    reverb_params.out_cross = (s16)args[13];
    reverb_params.input_gain = (s16)args[14];
    status = reverb_set(bus, &reverb_params);
    sources_routing_changed();
    return status;
}

static void reply_u32(u16 index, u32 value)
{
    BOX[EXT_REPLY + index] = (u16)(value >> 16);
    BOX[EXT_REPLY + index + 1] = (u16)value;
}

static u16 read_stats(vu16 *args)
{
    reply_u32(0, stats.frames);
    BOX[EXT_REPLY + 2] = stats.late;
    BOX[EXT_REPLY + 3] = stats.max_backlog;
    BOX[EXT_REPLY + 4] = stats.fifo_empty;
    reply_u32(5, stats.render_last);
    reply_u32(7, stats.render_max);
    reply_u32(9, stats.period_last);
    reply_u32(11, stats.period_min);
    reply_u32(13, stats.period_max);
    if (args[0])
        stats_reset_peaks();
    return EXT_OK;
}

static u16 handle(u16 command, vu16 *args)
{
    switch (command) {
    case EXT_CMD_PING:
        BOX[EXT_REPLY] = FIRMWARE_VERSION;
        return EXT_OK;
    case EXT_CMD_DELAY_SET:
        return set_delay(args);
    case EXT_CMD_REVERB_SET:
        return set_reverb(args);
    case EXT_CMD_STATS:
        return read_stats(args);
    }
    return EXT_ERR_COMMAND;
}

void mailbox_poll(void)
{
    u16 sequence = BOX[EXT_SEQUENCE];
    if (sequence == BOX[EXT_ACK])
        return;
    BOX[EXT_STATUS] = handle(BOX[EXT_COMMAND], BOX + EXT_ARGS);
    BOX[EXT_ACK] = sequence;
}
