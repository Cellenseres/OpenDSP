// SPDX-License-Identifier: BSD-2-Clause
#include <string.h>
#include "output.h"

Result opendsp_audio_output_open(audio_output *o, int channel, u32 count, u32 frames, const opendspAudioInfo *info)
{
    memset(o, 0, sizeof(*o));
    o->channel = channel;
    o->count = count;
    o->frames = frames;
    o->frame_bytes = info->channels * info->bits / 8;
    o->stride = (frames * o->frame_bytes + 31) & ~31u;
    o->bytes = o->stride * count;
    o->memory = linearMemAlign(o->bytes, 128);
    if (!o->memory) return OPENDSP_AUDIO_ERR_MEMORY;
    for (u32 i = 0; i < count; ++i) o->buffers[i].wave.data_vaddr = o->memory + i * o->stride;
    return 0;
}

void opendsp_audio_output_configure(audio_output *o, const opendspAudioInfo *info, float rate)
{
    u16 format = info->bits == 16 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_MONO_PCM8;
    if (info->channels == 2) format = info->bits == 16 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_STEREO_PCM8;
    opendsp_audio_output_pause(o, true);
    ndspChnSetFormat(o->channel, format);
    ndspChnSetRate(o->channel, rate);
}

void opendsp_audio_output_pause(audio_output *o, bool paused)
{
    if (o->pause_known && o->paused == paused) return;
    ndspChnSetPaused(o->channel, paused);
    o->paused = paused;
    o->pause_known = true;
}

void opendsp_audio_output_reap(audio_output *o)
{
    if (o->retiring || !o->queued) return;
    u16 seq = ndspChnGetWaveBufSeq(o->channel);
    u32 sample = ndspChnGetSamplePos(o->channel);
    for (u32 i = 0; i < o->count; ++i) {
        audio_buffer *b = &o->buffers[i];
        if (!b->queued) continue;
        u8 status = __atomic_load_n(&b->wave.status, __ATOMIC_ACQUIRE);
        if (status == NDSP_WBUF_DONE) {
            u64 end = b->start + b->wave.nsamples;
            if (end > o->completed) o->completed = end;
            b->queued = false;
            --o->queued;
            o->queued_frames -= b->wave.nsamples;
        } else if (b->wave.sequence_id == seq && status == NDSP_WBUF_PLAYING) {
            u64 position = b->start + (sample < b->wave.nsamples ? sample : b->wave.nsamples);
            if (position > o->position) o->position = position;
        }
    }
    if (o->completed > o->position) o->position = o->completed;
}

int opendsp_audio_output_free(const audio_output *o)
{
    if (o->retiring) return -1;
    for (u32 i = 0; i < o->count; ++i) if (!o->buffers[i].queued) return (int)i;
    return -1;
}
void *opendsp_audio_output_data(audio_output *o, int buffer) { return o->memory + buffer * o->stride; }

void opendsp_audio_output_queue(audio_output *o, int buffer, u32 frames)
{
    audio_buffer *b = &o->buffers[buffer];
    b->start = o->submitted;
    b->wave.nsamples = frames;
    b->wave.looping = false;
    b->queued = true;
    o->submitted += frames;
    ++o->queued;
    o->queued_frames += frames;
    DSP_FlushDataCache(opendsp_audio_output_data(o, buffer), (frames * o->frame_bytes + 31) & ~31u);
    ndspChnWaveBufAdd(o->channel, &b->wave);
    o->touched = true;
}

void opendsp_audio_output_retire(audio_output *o, u32 epoch)
{
    if (o->retiring || !o->touched) return;
    opendsp_audio_output_pause(o, true);
    ndspChnWaveBufClear(o->channel);
    o->retiring = true;
    o->fence_frame = ndspGetFrameCount();
    o->fence_epoch = epoch;
    o->fence_steps = 0;
}

bool opendsp_audio_output_fenced(audio_output *o, u32 epoch, bool shutdown)
{
    if (shutdown || !o->touched) return true;
    if (!o->retiring) return false;
    u32 frame = ndspGetFrameCount();
    if (o->fence_epoch != epoch) {
        o->fence_epoch = epoch;
        o->fence_frame = frame;
        o->fence_steps = 0;
    } else if (frame != o->fence_frame) {
        o->fence_frame = frame;
        ++o->fence_steps;
    }
    // Clear marks host wave buffers DONE before DSP has acknowledged the command.
    return o->fence_steps >= 4;
}

void opendsp_audio_output_reset(audio_output *o, u64 position)
{
    for (u32 i = 0; i < o->count; ++i) {
        memset(&o->buffers[i], 0, sizeof(o->buffers[i]));
        o->buffers[i].wave.data_vaddr = o->memory + i * o->stride;
    }
    o->queued = o->queued_frames = 0;
    o->submitted = o->completed = o->position = position;
    o->retiring = o->touched = false;
}

void opendsp_audio_output_close(audio_output *o)
{
    linearFree(o->memory);
    o->memory = NULL;
}
