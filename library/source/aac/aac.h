// SPDX-License-Identifier: BSD-2-Clause
// Portable fixed-point AAC-LC decoder core for ADTS streams.
#pragma once
#include <stddef.h>
#include <stdint.h>

#define AAC_FRAME_SAMPLES   1024
#define AAC_MAX_CHANNELS    2
#define AAC_MAX_SFB         51
#define AAC_MAX_WINDOWS     8
#define AAC_MAX_TNS_FILTERS 8
#define AAC_TNS_MAX_ORDER   12
#define AAC_MAX_BLOCKS      4

typedef enum {
    AAC_OK = 0,
    AAC_NEED_MORE_DATA,
    AAC_RESYNC,
    AAC_CORRUPT,
    AAC_UNSUPPORTED,
    AAC_OUTPUT_TOO_SMALL,
    AAC_INVALID_ARGUMENT,
} aac_status_t;

typedef struct {
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t blocks;         // raw data blocks in the frame
    uint8_t changed;        // rate or channels differ from the last frame
    uint8_t sbr;            // SBR data present and ignored
    uint8_t object_type;
    uint16_t frame_bytes;
} aac_frame_info_t;

typedef struct {
    uint8_t window_sequence, window_shape, max_sfb, num_swb;
    uint8_t num_windows, num_groups;
    uint8_t group_len[AAC_MAX_WINDOWS];
} aac_ics_t;

typedef struct {
    uint8_t window, bottom, top, order, direction;
    int16_t coef[AAC_TNS_MAX_ORDER];    // reflection coefficients, Q15
} aac_tns_filter_t;

typedef struct {
    aac_ics_t ics;
    uint8_t tns_count;
    uint8_t band_type[AAC_MAX_WINDOWS][AAC_MAX_SFB];    // [group][sfb]
    int16_t scalefactor[AAC_MAX_WINDOWS][AAC_MAX_SFB];
    aac_tns_filter_t tns[AAC_MAX_TNS_FILTERS];
    int32_t spec[AAC_FRAME_SAMPLES];                    // [window * 128 + k] for short windows
} aac_channel_t;

// Caller-owned state, one per stream and thread.
typedef struct {
    aac_channel_t ch[AAC_MAX_CHANNELS];
    int32_t overlap[AAC_MAX_CHANNELS][AAC_FRAME_SAMPLES];
    int32_t fft[AAC_FRAME_SAMPLES];
    int32_t time[2 * AAC_FRAME_SAMPLES];
    uint8_t ms_used[AAC_MAX_WINDOWS][AAC_MAX_SFB];
    uint8_t ms_mask_present;
    uint8_t prev_shape[AAC_MAX_CHANNELS];
    uint8_t sf_index, channels, configured, synced, sbr_seen;
    uint32_t noise_seed;
} aac_decoder_t;

void aac_decoder_init(aac_decoder_t *dec);

// Drops overlap and sync state, e.g. seeking.
void aac_decoder_reset(aac_decoder_t *dec);

// One ADTS frame per call; contract in opendsp/aac.h.
aac_status_t aac_decode(aac_decoder_t *dec, const uint8_t *in, size_t in_bytes, size_t *consumed,
                        int16_t *pcm, size_t pcm_frames, size_t *frames, aac_frame_info_t *info);
