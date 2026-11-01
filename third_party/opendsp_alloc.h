/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef OPENDSP_CODEC_ALLOC_H
#define OPENDSP_CODEC_ALLOC_H
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#ifdef _WIN32
#include <malloc.h>
#else
#include <alloca.h>
#endif

/* Decoder heap only. PCM output is accounted for by the streaming engine. */
typedef struct {
    size_t current, peak;
    void *allocations;
    char *opus_stack, *opus_scratch;
    bool failed;
    jmp_buf escape;
} audio_ogg_memory;
void *opendsp_codec_malloc(size_t size);
void *opendsp_codec_calloc(size_t count, size_t size);
void *opendsp_codec_realloc(void *ptr, size_t size);
void opendsp_codec_free(void *ptr);
void *opendsp_codec_scratch(size_t size);
void opendsp_audio_ogg_enter(audio_ogg_memory *memory);
void opendsp_audio_ogg_leave(void);
void opendsp_audio_ogg_release(void);
#ifdef OPENDSP_CODEC_ALLOCATOR
#define malloc opendsp_codec_malloc
#define calloc opendsp_codec_calloc
#define realloc opendsp_codec_realloc
#define free opendsp_codec_free
#undef alloca
#define alloca opendsp_codec_scratch
#endif
#endif
