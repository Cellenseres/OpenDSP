/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once
#include <opendsp/aac.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MPEG-1 Layer III, PCM16 output. AUTO selects DSP; ARM11 is explicit. */
#define OPENDSP_MP3_BACKEND_AUTO OPENDSP_AAC_BACKEND_AUTO
#define OPENDSP_MP3_BACKEND_ARM11 OPENDSP_AAC_BACKEND_ARM11
#define OPENDSP_MP3_BACKEND_DSP OPENDSP_AAC_BACKEND_DSP
#define OPENDSP_MP3_INFO_CHANGED OPENDSP_AAC_INFO_CHANGED
#define OPENDSP_MP3_INFO_DISCONTINUITY OPENDSP_AAC_INFO_DISCONTINUITY
#define OPENDSP_MP3_INFO_SKIPPED OPENDSP_AAC_INFO_SKIPPED
#define OPENDSP_MP3_ERR_NEED_MORE_DATA OPENDSP_AAC_ERR_NEED_MORE_DATA
#define OPENDSP_MP3_ERR_CORRUPT OPENDSP_AAC_ERR_CORRUPT
#define OPENDSP_MP3_ERR_UNSUPPORTED OPENDSP_AAC_ERR_UNSUPPORTED
#define OPENDSP_MP3_ERR_OUTPUT_TOO_SMALL OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL
#define OPENDSP_MP3_ERR_INVALID_ARG OPENDSP_AAC_ERR_INVALID_ARG
#define OPENDSP_MP3_ERR_NO_MEMORY OPENDSP_AAC_ERR_NO_MEMORY
#define OPENDSP_MP3_ERR_BACKEND OPENDSP_AAC_ERR_BACKEND
#define OPENDSP_MP3_ERR_BUSY OPENDSP_AAC_ERR_BUSY
#define OPENDSP_MP3_ERR_TIMEOUT OPENDSP_AAC_ERR_TIMEOUT
#define OPENDSP_MP3_ERR_CRC MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_DSP, 0x2C0)

typedef struct opendspMp3Decoder opendspMp3Decoder;
typedef opendspAacConfig opendspMp3Config;
typedef struct {
    u32 struct_size;
    u32 frames, errors, yields, stack_words;
    u32 last_ticks, worst_ticks, worst_slice_ticks;
    u32 last_latency_ticks, worst_latency_ticks;
    /* Active ticks: parse, Huffman, requantize, hybrid, DCT32, synthesis. */
    u32 stage_ticks[6];
} opendspMp3Stats;
typedef struct {
    u32 sample_rate;
    u16 channels, frame_samples;
    u16 bitrate_kbps, reserved;
    u32 flags;
} opendspMp3Info;

Result opendspMp3Open(opendspMp3Decoder **out, const opendspMp3Config *config);
Result opendspMp3Decode(opendspMp3Decoder *decoder, const void *input, size_t bytes, size_t *consumed,
                        s16 *pcm, size_t capacity_frames, size_t *frames, opendspMp3Info *info);
void opendspMp3Reset(opendspMp3Decoder *decoder);
void opendspMp3Close(opendspMp3Decoder *decoder);
u32 opendspMp3GetBackend(const opendspMp3Decoder *decoder);
/* GetStats returns DSP timer/stack counters; ARM11 returns ERR_BACKEND. */
Result opendspMp3GetStats(opendspMp3Decoder *decoder, opendspMp3Stats *stats);

#ifdef __cplusplus
}
#endif
