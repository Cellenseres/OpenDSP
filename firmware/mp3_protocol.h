/* SPDX-License-Identifier: BSD-2-Clause
 * Additive MP3 transport. Layout shares control/input geometry with AAC;
 * 4608-byte aligned PCM slots hold MPEG-1's 1152 stereo PCM16 sample frames.
 */
#pragma once
#include "aac_protocol.h"
#include "mp3/table_layout.h"
#define MP3_ABI 1
#define MP3_MAGIC 0x4d33
#define MP3_PCM_BYTES 4608
#define MP3_SHARED_BYTES (AAC_PCM_OFFSET + AAC_SLOTS * MP3_PCM_BYTES)
#define MP3_CMD_OPEN 0x30
#define MP3_CMD_CLOSE 0x31
#define MP3_CMD_RESET 0x32
#define MP3_CMD_STATS 0x33
#define MP3_CAP_DSP (1u << 5)

/* Byte-stream ABI. ARM and DSP own separate DSP-RAM records. Output descriptors
 * select only validated slots in the same allocation; no arbitrary DMA target.
 * Input positions are modulo 65536; occupancy must never exceed 32768 bytes. */
#define MP3_STREAM_ABI 2
#define MP3_CAP_STREAM (1u << 6)
#define MP3_CMD_STREAM_OPEN 0x34
#define MP3_EVENT_PIPE 4
#define MP3_STREAM_SLOTS 8
#define MP3_STREAM_STATUS 64
#define MP3_STREAM_RESULTS 128
#define MP3_STREAM_DESCRIPTORS 640
#define MP3_STREAM_META_WORD 0x0d40
#define MP3_STREAM_META_BYTES 1152
#define MP3_STREAM_INPUT 0
#define MP3_STREAM_INPUT_BYTES 32768
#define MP3_STREAM_PCM (MP3_STREAM_INPUT + MP3_STREAM_INPUT_BYTES)
#define MP3_STREAM_REFILL 16384
#define MP3_STREAM_MIN_FRAMES 2304
#define MP3_STREAM_SYNC_LIMIT 4096

#if MP3_STREAM_META_WORD + MP3_STREAM_META_BYTES / 2 > 0x0f80
#error MP3 metadata overlaps codec task registers
#endif
#define MP3_STREAM_HEADER 1
#define MP3_STREAM_NEED_INPUT 2
#define MP3_STREAM_DRAINED 4
#define MP3_STREAM_ERROR 8
#define MP3_STREAM_PROFILE 1
#define MP3_STREAM_DOORBELL 0x1000
