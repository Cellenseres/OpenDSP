/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once
#include "codec.h"
#include "../../../third_party/opendsp_alloc.h"

#define AUDIO_OGG_PACKET_LIMIT (1024u * 1024u)
#define AUDIO_OGG_COMMENT_LIMIT (256u * 1024u)
#define AUDIO_OGG_COMMENT_COUNT 4096u

typedef struct {
    u64 start, end, length;
    bool has_start, has_end, has_length, invalid, present;
} audio_loop_tags;
void audio_loop_tag_init(audio_loop_tags *tags);
void audio_loop_tag_add(audio_loop_tags *tags, const char *comment, size_t length);
void audio_loop_tag_finish(const audio_loop_tags *tags, u64 total, opendspAudioLoopInfo *info);

typedef struct {
    audio_codec *codec;
    Result error;
    audio_ogg_memory memory;
} audio_ogg_io;
/* Keep the jump target frame alive. */
#define AUDIO_OGG_ENTER(io) do { \
    opendsp_audio_ogg_enter(&(io)->memory); \
    if ((io)->memory.failed) { \
        opendsp_audio_ogg_leave(); \
        return OPENDSP_AUDIO_ERR_MEMORY; \
    } \
    if (setjmp((io)->memory.escape)) { \
        opendsp_audio_ogg_leave(); \
        return OPENDSP_AUDIO_ERR_MEMORY; \
    } \
} while (0)
int audio_ogg_read(audio_ogg_io *io, void *out, size_t bytes);
int audio_ogg_seek(audio_ogg_io *io, s64 offset, int whence);
Result audio_ogg_error(audio_ogg_io *io, Result fallback);
/* DSP Vorbis setup started by inspect. */
typedef struct {
    struct aac_dsp_backend *dsp;
    u8 *headers[2];
    size_t header_bytes[2];
    Result result;
    bool pending;
} audio_vorbis_start;
void audio_vorbis_start_dsp(audio_vorbis_start *s, audio_codec *c, const u8 *id, const u8 *setup, size_t bytes);
void audio_vorbis_start_cancel(audio_vorbis_start *s, audio_codec *c);
Result opendsp_audio_vorbis_open(audio_codec *c, const audio_loop_tags *tags, u32 backend,
                                 audio_vorbis_start *start);
Result opendsp_audio_opus_open(audio_codec *c, const audio_loop_tags *tags, u32 backend);
