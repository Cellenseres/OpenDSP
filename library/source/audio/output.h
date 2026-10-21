// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "codec.h"

typedef struct {
    ndspWaveBuf wave;
    u64 start;
    bool queued;
    bool pending;
    u16 ticket;
} audio_buffer;

typedef struct audio_output {
    audio_buffer buffers[8];
    u8 *memory;
    u32 count, frames, stride, frame_bytes, bytes;
    int channel;
    u32 queued, queued_frames;
    u16 played;
    u64 submitted, completed, position;
    bool touched, retiring, pause_known, paused;
    bool borrowed;
    u32 fence_frame, fence_epoch, fence_steps;
} audio_output;

Result opendsp_audio_output_open(audio_output *o, int channel, u32 count, u32 frames,
                                 const opendspAudioInfo *info, void *borrowed, u32 stride);
void opendsp_audio_output_configure(audio_output *o, const opendspAudioInfo *info, float rate);
void opendsp_audio_output_pause(audio_output *o, bool paused);
void opendsp_audio_output_reap(audio_output *o);
int opendsp_audio_output_free(const audio_output *o);
void *opendsp_audio_output_data(audio_output *o, int buffer);
void opendsp_audio_output_queue(audio_output *o, int buffer, u32 frames);
void opendsp_audio_output_retire(audio_output *o, u32 epoch);
bool opendsp_audio_output_fenced(audio_output *o, u32 epoch, bool shutdown);
void opendsp_audio_output_reset(audio_output *o, u64 position);
void opendsp_audio_output_close(audio_output *o);
