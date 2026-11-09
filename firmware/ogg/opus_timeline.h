// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"
#include <stdint.h>

// Use decoder task arithmetic mode.

typedef struct {
    uint64_t decoded_frame, first_raw, last_raw, source_frame;
    u16 pre_skip, failed;
} ogg_opus_timeline;

typedef struct {
    uint64_t source_frame;
    u16 skip, frames;
} ogg_opus_span;

int ogg_opus_timeline_open(ogg_opus_timeline *timeline, u16 pre_skip, uint64_t total,
                           uint64_t first, uint64_t last, uint64_t decode_start);
int ogg_opus_timeline_slice(ogg_opus_timeline *timeline, u16 frames, ogg_opus_span *span);
int ogg_opus_timeline_finish(ogg_opus_timeline *timeline);
