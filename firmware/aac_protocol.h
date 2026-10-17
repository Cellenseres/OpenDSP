// SPDX-License-Identifier: BSD-2-Clause
// Versioned single-stream transport. *_BYTES / *_OFFSET are byte counts;
// control/result indices in the layout comments below are 16-bit words.
#pragma once
#define AAC_ABI 1
#define AAC_MAGIC 0x4143
#define AAC_SLOTS 4
#define AAC_CONTROL_BYTES 256
#define AAC_INPUT_BYTES 2048
#define AAC_PCM_BYTES 4096
#define AAC_INPUT_OFFSET 256
#define AAC_PCM_OFFSET (AAC_INPUT_OFFSET + AAC_SLOTS * AAC_INPUT_BYTES)
#define AAC_SHARED_BYTES (AAC_PCM_OFFSET + AAC_SLOTS * AAC_PCM_BYTES)
#define AAC_TABLE_WORDS 11358
#define AAC_TABLE_ADDRESS 0x1C000
#define AAC_CMD_OPEN 0x20
#define AAC_CMD_CLOSE 0x21
#define AAC_CMD_RESET 0x22
#define AAC_CMD_STATS 0x23
#define AAC_RESOURCE_BUSY 7
#define AAC_BAD_TRANSPORT 8
#define AAC_CAP_DSP (1u << 4)
// ARM: magic, ABI, session, producer; lengths at 16 + slot*4.
// DSP: result records at 64 + slot*16: sequence (commit last), status,
// channels, sampling index, SBR, changed, decode ticks hi/lo, session.
