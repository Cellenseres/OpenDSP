// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>
#include <stdio.h>
#include <opendsp/audio.h>

typedef struct {
    FILE *file;
    u64 size;
    const u32 *generation;
    u32 expected;
    opendspAudioStats *stats;
} audio_input;

Result opendsp_audio_input_open(audio_input *in, const char *path);
Result opendsp_audio_input_seek(audio_input *in, u64 offset);
Result opendsp_audio_input_read(audio_input *in, void *data, size_t size, size_t *got);
bool opendsp_audio_input_cancelled(const audio_input *in);

typedef struct audio_codec audio_codec;
typedef struct {
    Result (*read)(audio_codec *, void *, u32, u32 *);
    Result (*seek)(audio_codec *, u64);
    void (*close)(audio_codec *);
} audio_codec_ops;

struct audio_codec {
    audio_input in;
    const audio_codec_ops *ops;
    opendspAudioInfo info;
    opendspAudioStats stats;
    u32 backend, memory;
    u64 position;
    void *data;
};
Result opendsp_audio_codec_open(audio_codec *c, const char *path, u32 backend);
void opendsp_audio_codec_close(audio_codec *c);
Result opendsp_audio_wav_open(audio_codec *c);
Result opendsp_audio_aac_open(audio_codec *c, u32 backend);
