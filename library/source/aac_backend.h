// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "opendsp/aac.h"
#include "opendsp/mp3.h"
typedef struct aac_dsp_backend aac_dsp_backend;
Result aac_backend_open(aac_dsp_backend **out);
Result codec_backend_open(aac_dsp_backend **out, u16 codec);
Result mp3_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                          size_t capacity, size_t *frames, opendspMp3Info *info);
Result aac_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                          size_t capacity, size_t *frames, opendspAacInfo *info);
void aac_backend_reset(aac_dsp_backend *d);
void aac_backend_close(aac_dsp_backend *d);
Result aac_backend_stats(aac_dsp_backend *d, opendspAacStats *stats);
Result mp3_backend_stages(aac_dsp_backend *d, u32 ticks[6]);
void opendsp_aac_setup(void);
void opendsp_aac_release(void);

/* Internal asynchronous MP3 transport. PCM slots are part of the quarantinable
 * arena and lent to NDSP; ARM never reads, copies or flushes their contents. */
typedef struct {
    u16 read, completed, flags, error, rate, channels;
    u32 skipped;
} mp3_stream_progress;
Result mp3_backend_stream_open(aac_dsp_backend **out, u16 count, u16 frames, bool profile);
Result mp3_backend_stream_poll(aac_dsp_backend *d, mp3_stream_progress *p);
u8 *mp3_backend_stream_input(aac_dsp_backend *d, u16 read, size_t *available);
void mp3_backend_stream_written(aac_dsp_backend *d, size_t bytes, bool eof);
Result mp3_backend_stream_submit(aac_dsp_backend *d, u16 slot, u16 *ticket);
Result mp3_backend_stream_result(aac_dsp_backend *d, u16 ticket, u16 *frames);
void mp3_backend_stream_commit(aac_dsp_backend *d);
void mp3_backend_stream_played(aac_dsp_backend *d, u16 count);
void *mp3_backend_stream_pcm(aac_dsp_backend *d, u32 *stride);
u32 mp3_backend_stream_overhead(void);
Result mp3_backend_stream_reset(aac_dsp_backend *d, bool preserve_output);
void mp3_backend_stream_stop(aac_dsp_backend *d);
