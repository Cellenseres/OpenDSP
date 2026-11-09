// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>
#include <stdio.h>
#include <opendsp/audio.h>

#define AUDIO_INPUT_BLOCK 32768u
// Ogg open touches four blocks.
#define AUDIO_INPUT_BLOCKS 4u

typedef struct {
    FILE *file;
    u64 size;
    u64 at, file_at; // logical, stdio
    u64 block_at[AUDIO_INPUT_BLOCKS];
    u8 *block;
    u32 block_bytes[AUDIO_INPUT_BLOCKS], block_used[AUDIO_INPUT_BLOCKS], clock;
    const u32 *generation;
    u32 expected;
    opendspAudioStats *stats;
} audio_input;

Result opendsp_audio_input_open(audio_input *in, const char *path);
Result opendsp_audio_input_seek(audio_input *in, u64 offset);
Result opendsp_audio_input_read(audio_input *in, void *data, size_t size, size_t *got);
void opendsp_audio_input_close(audio_input *in);
static inline u64 opendsp_audio_input_tell(const audio_input *in) { return in->at; }
static inline bool opendsp_audio_input_eof(const audio_input *in) { return in->at >= in->size; }
bool opendsp_audio_input_cancelled(const audio_input *in);

typedef struct audio_codec audio_codec;
typedef struct {
    Result (*read)(audio_codec *, void *, u32, u32 *);
    Result (*seek)(audio_codec *, u64);
    void (*close)(audio_codec *);
    Result (*stats)(audio_codec *, opendspAudioDecoderStats *);
} audio_codec_ops;

struct audio_codec {
    audio_input in;
    const audio_codec_ops *ops;
    opendspAudioInfo info;
    opendspAudioStats stats;
    opendspAudioLoopInfo loop;
    bool seekable;
    u32 backend, memory;
    u64 position;
    void *data;
    void *direct_pcm;
    u32 direct_stride;
    bool first_toc_known;
    u8 first_toc;
};
// DSP MP3 default: 8 buffers, ~190 ms
#define AUDIO_MP3_DSP_BUFFERS 8
Result opendsp_audio_codec_open(audio_codec *c, const char *path, u32 backend, u32 count, u32 frames,
                                u32 flags);
void opendsp_audio_codec_close(audio_codec *c);
Result opendsp_audio_wav_open(audio_codec *c);
Result opendsp_audio_ogg_open(audio_codec *c, u32 backend);
Result opendsp_audio_aac_open(audio_codec *c, u32 backend);
Result opendsp_audio_mp3_open(audio_codec *c, u32 backend, u32 count, u32 frames, u32 flags);
Result opendsp_audio_mp3_arm11_open(audio_codec *c);
struct audio_output;
Result opendsp_audio_mp3_pump(audio_codec *c, struct audio_output *out, bool *progress, bool *eof);
void opendsp_audio_mp3_stop(audio_codec *c);
void opendsp_audio_mp3_touch(audio_codec *c);
