// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

#define OGG_PCM_SLOTS 8
// Output state stays in X memory.
typedef struct {
    u16 bounce[128];
    u32 address[OGG_PCM_SLOTS], ready_frames;
    u16 lengths[OGG_PCM_SLOTS];
    u16 slots, frames, channels, index, used;
    u16 produced, consumed, failed, finished;
} ogg_pcm_output;

int ogg_pcm_open(ogg_pcm_output *output, u32 begin, u32 end, u16 slots, u16 frames, u16 channels);
int ogg_pcm_write(ogg_pcm_output *output, const s16 *pcm, u16 frames);
int ogg_pcm_finish(ogg_pcm_output *output);
int ogg_pcm_ack(ogg_pcm_output *output, u16 consumed);
int ogg_pcm_slot(const ogg_pcm_output *output, u16 sequence, u32 *address, u16 *frames);
