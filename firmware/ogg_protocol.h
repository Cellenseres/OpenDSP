// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#define OGG_ABI 1
#define OGG_CAP_OPUS_DSP (1u << 8)
#define OGG_CAP_VORBIS_DSP (1u << 9)
// Mailbox: state, result, frames, channels.
// Then the finished ticket, written last.
#define OGG_CAP_DONE_WORD (1u << 10)
#define OGG_DONE_STATUS 59
#define OGG_DONE_WORD 63
#define OGG_CODEC_VORBIS 0x100u
#define OGG_CMD_OPEN 0x38
#define OGG_CMD_INIT 0x39
#define OGG_CMD_DECODE 0x3a
#define OGG_CMD_STATUS 0x3b
#define OGG_CMD_CLOSE 0x3c
#define OGG_CMD_STATS 0x3d
#define OGG_CMD_RESET 0x3e
#define OGG_INPUT_BYTES 65536UL
#define OGG_PCM_FRAMES 5760u
#define OGG_PCM_BYTES 23040UL
#define OGG_STATE_PREPARED 0
#define OGG_STATE_BUSY 1
#define OGG_STATE_DONE 2
#define OGG_STATE_FAILED 3
// OPEN: ABI, session, pool, channels.
// Then arena begin/end, input, PCM.
// INIT: session, ticket.
// DECODE: session, ticket, bytes, frames, FEC.
// Then signed gain in Q8 dB.
// STATUS: ticket, state, result, frames, channels.
// Then duration, range hi/lo.
