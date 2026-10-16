// SPDX-License-Identifier: BSD-2-Clause
// OpenDSP extension mailbox, shared with libopendsp.
#pragma once

// DSP data word address; ARM view via DSP_ConvertProcessAddressFromDspDram
#define EXT_BASE        0x0D00
#define EXT_WORDS       0x0040

// header, written by the DSP at boot
#define EXT_MAGIC0      0       // 'OD'
#define EXT_MAGIC1      1       // 'SP'
#define EXT_PROTOCOL    2
#define EXT_VERSION     3       // major << 8 | minor
#define EXT_CAPS_LO     4
#define EXT_CAPS_HI     5
#define EXT_ACK         6       // last handled sequence
#define EXT_STATUS      7       // result of that request
// request, written by the ARM; sequence last
#define EXT_SEQUENCE    8
#define EXT_COMMAND     9
#define EXT_ARGS        10
#define EXT_ARG_WORDS   32
// reply words, valid once EXT_ACK matches
#define EXT_REPLY       42
#define EXT_REPLY_WORDS 16

#define EXT_MAGIC0_VALUE 0x4F44
#define EXT_MAGIC1_VALUE 0x5350
#define EXT_PROTOCOL_VERSION 1

// capability bits; the others are reserved
#define EXT_CAP_MAILBOX  (1u << 0)
#define EXT_CAP_DELAY    (1u << 1)
#define EXT_CAP_REVERB   (1u << 2)

enum {
    EXT_CMD_PING = 0,
    EXT_CMD_DELAY_SET = 1,      // bus, enable, frames, g, a, b, address hi, lo, zeroed
    EXT_CMD_REVERB_SET = 2,     // see EXT_REVERB_* below
    EXT_CMD_STATS = 3,          // reset peaks; reply: frames hi, lo, late, max backlog,
                                // fifo empty, render hi, lo, render max hi, lo, period hi, lo,
                                // period min hi, lo, period max hi, lo
    EXT_CMD_COUNT
};

// reverb arguments: bus, enable, 4 line lengths, 4 feedback gains,
// damping, diffusion, output same side, output other side, input gain (Q15)
#define EXT_REVERB_ARG_WORDS 15
#define EXT_REVERB_LINE_MAX  { 1777, 1543, 1291, 1009 }
#define EXT_REVERB_LINE_MIN  160

enum {
    EXT_OK = 0,
    EXT_ERR_COMMAND = 1,
    EXT_ERR_ARGUMENT = 2,
    EXT_ERR_ADDRESS = 3,
    EXT_ERR_UNSUPPORTED = 4,
};

// FCRAM range the DSP may DMA to
#define EXT_FCRAM_START 0x20000000UL
#define EXT_FCRAM_END   0x28000000UL
