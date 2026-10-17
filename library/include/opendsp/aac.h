// SPDX-License-Identifier: BSD-2-Clause
// AAC-LC (ADTS) decoder producing PCM16 for ndsp.
#pragma once
#include <stddef.h>
#include <3ds/types.h>
#include <3ds/result.h>

#ifdef __cplusplus
extern "C" {
#endif

// AAC-LC in ADTS frames: mono or stereo, 8000 to 96000 Hz, 1024 samples per
// channel and raw data block. One decoder per stream, used by one thread at a time.
//
// opendspAacDecode() handles at most one frame per call:
//   0                                 frame decoded
//   OPENDSP_AAC_INFO_SKIPPED          junk before the next sync word skipped,
//                                     nothing written; counts as success
//   OPENDSP_AAC_ERR_NEED_MORE_DATA    no complete frame yet, nothing consumed
//   OPENDSP_AAC_ERR_CORRUPT           damaged frame skipped, decoding goes on
//   OPENDSP_AAC_ERR_UNSUPPORTED       frame skipped: not LC, more than two
//                                     channels, 7350 Hz, CCE/LFE, gain control
//   OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL  pcm_capacity_frames < 1024 * info->blocks
//   OPENDSP_AAC_ERR_INVALID_ARG       NULL pointer or bad config
//
// info is filled whenever a header was read, also on errors. INFO_CHANGED marks
// a new rate or channel count. HE-AAC plays as its AAC-LC core at half the rate,
// flagged with INFO_SBR_IGNORED. Stereo output is interleaved. CRC words are
// skipped, not checked.

#define OPENDSP_AAC_BACKEND_AUTO  0     // the ARM11 for now
#define OPENDSP_AAC_BACKEND_ARM11 1
#define OPENDSP_AAC_BACKEND_DSP   2     // the firmware has no decoder: ERR_BACKEND

#define OPENDSP_AAC_INFO_CHANGED      (1u << 0)
#define OPENDSP_AAC_INFO_SBR_IGNORED  (1u << 1)   // HE-AAC: only the LC core decoded

#define OPENDSP_AAC_INFO_SKIPPED          MAKERESULT(RL_INFO, RS_NOP, RM_DSP, 0x2A1)
#define OPENDSP_AAC_ERR_NEED_MORE_DATA    MAKERESULT(RL_STATUS, RS_WOULDBLOCK, RM_DSP, 0x2A2)
#define OPENDSP_AAC_ERR_CORRUPT           MAKERESULT(RL_PERMANENT, RS_WRONGARG, RM_DSP, 0x2A3)
#define OPENDSP_AAC_ERR_UNSUPPORTED       MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, 0x2A4)
#define OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL  MAKERESULT(RL_USAGE, RS_OUTOFRESOURCE, RM_DSP, 0x2A5)
#define OPENDSP_AAC_ERR_INVALID_ARG       MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_DSP, 0x2A6)
#define OPENDSP_AAC_ERR_NO_MEMORY         MAKERESULT(RL_PERMANENT, RS_OUTOFRESOURCE, RM_DSP, 0x2A7)
#define OPENDSP_AAC_ERR_BACKEND           MAKERESULT(RL_PERMANENT, RS_NOTSUPPORTED, RM_DSP, 0x2A8)

typedef struct opendspAacDecoder opendspAacDecoder;

typedef struct {
    u32 struct_size;    // sizeof(opendspAacConfig)
    u32 backend;        // OPENDSP_AAC_BACKEND_*
    u32 flags;          // reserved, 0
} opendspAacConfig;

typedef struct {
    u32 sample_rate;
    u16 channels;
    u16 frame_samples;  // per channel and raw data block, 1024
    u16 blocks;         // raw data blocks in the frame
    u16 object_type;    // from the ADTS header, 2 = LC
    u32 flags;          // OPENDSP_AAC_INFO_*
} opendspAacInfo;

// config may be NULL for defaults. The decoder needs about 32 KiB of heap.
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
