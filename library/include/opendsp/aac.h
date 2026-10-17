// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <stddef.h>
#include <3ds/types.h>
#include <3ds/result.h>

#ifdef __cplusplus
extern "C" {
#endif

// AAC-LC in ADTS frames: mono or stereo, 1024 samples per channel and block.
// ARM11: 8000 to 96000 Hz, 1..4 blocks. DSP: 8000 to 48000 Hz,
// one block, at most 2048 encoded bytes; requires a running OpenDSP instance.
// One decoder per stream, used by one thread at a time. DSP has one owner.
//
// Decode handles one ADTS frame per call. NEED_MORE_DATA and OUTPUT_TOO_SMALL
// consume nothing. SKIPPED, CORRUPT and UNSUPPORTED consume the rejected input.
// info is filled after a header is read, including on errors. HE-AAC decodes its
// LC core at half rate and sets INFO_SBR_IGNORED. CRC words are not checked.

#define OPENDSP_AAC_BACKEND_AUTO  0     // prefer DSP, otherwise ARM11
#define OPENDSP_AAC_BACKEND_ARM11 1
#define OPENDSP_AAC_BACKEND_DSP   2

#define OPENDSP_AAC_INFO_CHANGED      (1u << 0)
#define OPENDSP_AAC_INFO_SBR_IGNORED  (1u << 1)   // HE-AAC: only the LC core decoded
#define OPENDSP_AAC_INFO_DISCONTINUITY (1u << 2) // DSP context was lost across sleep/reload

#define OPENDSP_AAC_INFO_SKIPPED          MAKERESULT(RL_INFO, RS_NOP, RM_DSP, 0x2A1)
#define OPENDSP_AAC_ERR_NEED_MORE_DATA    MAKERESULT(RL_STATUS, RS_WOULDBLOCK, RM_DSP, 0x2A2)
#define OPENDSP_AAC_ERR_CORRUPT           MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_DSP, 0x2A3)
#define OPENDSP_AAC_ERR_UNSUPPORTED       MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, 0x2A4)
#define OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL  MAKERESULT(RL_USAGE, RS_OUTOFRESOURCE, RM_DSP, 0x2A5)
#define OPENDSP_AAC_ERR_INVALID_ARG       MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_DSP, 0x2A6)
#define OPENDSP_AAC_ERR_NO_MEMORY         MAKERESULT(RL_PERMANENT, RS_OUTOFRESOURCE, RM_DSP, 0x2A7)
#define OPENDSP_AAC_ERR_BACKEND           MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, 0x2A8)
#define OPENDSP_AAC_ERR_BUSY              MAKERESULT(RL_STATUS, RS_OUTOFRESOURCE, RM_DSP, 0x2A9)
#define OPENDSP_AAC_ERR_TIMEOUT           MAKERESULT(RL_STATUS, RS_WOULDBLOCK, RM_DSP, 0x2AA)

typedef struct opendspAacDecoder opendspAacDecoder;

typedef struct {
    u32 struct_size;    // sizeof(opendspAacConfig)
    u32 backend;        // OPENDSP_AAC_BACKEND_*
    u32 flags;          // reserved, use 0
} opendspAacConfig;

typedef struct {
    u32 sample_rate;
    u16 channels;
    u16 frame_samples;  // per channel and raw data block, 1024
    u16 blocks;         // raw data blocks in the frame
    u16 object_type;    // from the ADTS header, 2 = LC
    u32 flags;          // OPENDSP_AAC_INFO_*
} opendspAacInfo;

typedef struct {
    u32 struct_size;
    u32 frames, errors, yields, stack_words;
    u32 last_ticks, worst_ticks, worst_slice_ticks;
    u32 last_latency_ticks, worst_latency_ticks;
} opendspAacStats;

u32 opendspAacGetBackend(const opendspAacDecoder *dec);
Result opendspAacGetStats(opendspAacDecoder *dec, opendspAacStats *stats);

// NULL config / AUTO tries DSP, then ARM11 if opening DSP fails.
// Selection is fixed until close; choose ARM11 for rates above 48 kHz or multi-block frames.
// DSP uses a 24832-byte linear arena; ARM11 uses about 32 KiB of heap.
Result opendspAacOpen(opendspAacDecoder **out, const opendspAacConfig *config);

Result opendspAacDecode(opendspAacDecoder *dec, const void *input, size_t input_bytes, size_t *bytes_consumed,
                        s16 *pcm_interleaved, size_t pcm_capacity_frames, size_t *frames_written,
                        opendspAacInfo *info);

// Forget overlap and sync state, e.g. after seeking.
void opendspAacReset(opendspAacDecoder *dec);

void opendspAacClose(opendspAacDecoder *dec);

#ifdef __cplusplus
}
#endif
