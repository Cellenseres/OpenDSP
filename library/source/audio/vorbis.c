/* SPDX-License-Identifier: BSD-2-Clause */
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ivorbisfile.h>
#include "ogg.h"
#ifdef OPENDSP_CODEC_BANKS
#include "../ogg_backend.h"
#include "ogg_protocol.h"
#endif

typedef struct {
    audio_ogg_io io;
    OggVorbis_File file;
    bool opened;
#ifdef OPENDSP_CODEC_BANKS
    aac_dsp_backend *dsp;
    u8 *headers[2];
    size_t header_bytes[2];
    u32 decode_generation;
    bool dsp_faulted, headers_pending;
#endif
} vorbis_data;

static u32 vorbis_memory(const vorbis_data *v)
{
    u32 bytes = sizeof(*v) + v->io.memory.peak;
#ifdef OPENDSP_CODEC_BANKS
    bytes += v->header_bytes[0] + v->header_bytes[1];
    if (v->dsp) bytes += aac_backend_memory(v->dsp);
#endif
    return bytes;
}

#ifdef OPENDSP_CODEC_BANKS
static Result submit_headers(aac_dsp_backend *dsp, u8 *const headers[2], const size_t header_bytes[2],
                             audio_input *input, bool wait)
{
    static const u8 comment[16] = {3,'v','o','r','b','i','s',0,0,0,0,0,0,0,0,1};
    const u8 *packets[3] = {headers[0], comment, headers[1]};
    size_t bytes[3] = {header_bytes[0], sizeof(comment), header_bytes[1]};
    s16 scratch[2];
    for (unsigned index = 0; index < 3; ++index) {
        u16 frames = 0;
        if (index == 2 && !wait)
            return vorbis_backend_send(dsp, packets[2], bytes[2]);
        Result result = vorbis_backend_packet(dsp, packets[index], bytes[index], -1, false,
                                              scratch, 1, &frames, input->generation, input->expected);
        if (R_FAILED(result)) return result;
        if (frames) return OPENDSP_AUDIO_ERR_FORMAT;
    }
    return 0;
}

static Result finish_headers(aac_dsp_backend *dsp, bool *pending, audio_input *input)
{
    if (!*pending) return 0;
    *pending = false;
    s16 scratch[2];
    u16 frames = 0;
    Result result = vorbis_backend_receive(dsp, scratch, 1, &frames, input->generation, input->expected);
    return R_SUCCEEDED(result) && frames ? OPENDSP_AUDIO_ERR_FORMAT : result;
}

void audio_vorbis_start_dsp(audio_vorbis_start *s, audio_codec *c, const u8 *id, const u8 *setup, size_t bytes)
{
    Result result = OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (bytes >= OGG_INPUT_BYTES) goto done;
    result = OPENDSP_AUDIO_ERR_MEMORY;
    s->headers[0] = malloc(30);
    s->headers[1] = malloc(bytes);
    if (!s->headers[0] || !s->headers[1]) goto done;
    memcpy(s->headers[0], id, 30);
    memcpy(s->headers[1], setup, bytes);
    s->header_bytes[0] = 30;
    s->header_bytes[1] = bytes;
    result = vorbis_backend_open(&s->dsp, id[11]);
    if (R_SUCCEEDED(result)) result = submit_headers(s->dsp, s->headers, s->header_bytes, &c->in, false);
    s->pending = R_SUCCEEDED(result);
done:
    s->result = result;
}

void audio_vorbis_start_cancel(audio_vorbis_start *s, audio_codec *c)
{
    finish_headers(s->dsp, &s->pending, &c->in);
    if (s->dsp) aac_backend_close(s->dsp);
    free(s->headers[0]);
    free(s->headers[1]);
    memset(s, 0, sizeof(*s));
}

static int decode_callback(void *context, const ogg_packet *packet, ogg_int16_t *pcm, int capacity, int channels)
{
    vorbis_data *v = context;
    audio_input *input = &v->io.codec->in;
    Result result;
    u16 frames = 0;
    if (channels != v->io.codec->info.channels || capacity < 1 || capacity > 4096 ||
        packet->bytes <= 0 || (u64)packet->bytes >= OGG_INPUT_BYTES) {
        v->io.error = OPENDSP_AUDIO_ERR_UNSUPPORTED;
        return OV_EBADPACKET;
    }
    u32 generation = ov_decode_generation(&v->file);
    result = finish_headers(v->dsp, &v->headers_pending, input);
    if (R_FAILED(result)) goto failed;
    if (v->dsp_faulted) {
        result = ogg_backend_reset(v->dsp, input->generation, input->expected);
        if (R_SUCCEEDED(result)) result = submit_headers(v->dsp, v->headers, v->header_bytes, input, true);
        if (R_FAILED(result)) goto failed;
        v->dsp_faulted = false;
        v->decode_generation = generation;
    } else if (generation != v->decode_generation) {
        result = vorbis_backend_restart(v->dsp, input->generation, input->expected);
        if (R_FAILED(result)) goto failed;
        v->decode_generation = generation;
    }
    result = vorbis_backend_packet(v->dsp, packet->packet, packet->bytes, packet->granulepos, packet->e_o_s != 0,
                                  pcm, capacity, &frames, input->generation, input->expected);
    if (R_SUCCEEDED(result)) return frames;
failed:
    v->dsp_faulted = true;
    v->io.error = result;
    return OV_EBADPACKET;
}

static Result connect_dsp(vorbis_data *v)
{
    AUDIO_OGG_ENTER(&v->io);
    Result result = 0;
    if (ov_set_decode_callback(&v->file, decode_callback, v))
        result = OPENDSP_AUDIO_ERR_MEMORY;
    v->decode_generation = ov_decode_generation(&v->file);
    opendsp_audio_ogg_leave();
    return result;
}

static Result vorbis_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    vorbis_data *v = c->data;
    if (!v->dsp) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (!ogg_backend_context_valid(v->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
    opendspAacStats native = {.struct_size = sizeof(native)};
    Result result = aac_backend_stats(v->dsp, &native);
    if (R_FAILED(result)) return result;
    memset(stats, 0, sizeof(*stats));
    memcpy(stats, &native, sizeof(native));
    stats->struct_size = sizeof(*stats);
    return 0;
}
#endif

static size_t read_callback(void *out, size_t size, size_t count, void *context)
{
    audio_ogg_io *io = context;
    if (!size || !count)
        return 0;
    if (count > INT_MAX / size) {
        io->error = OPENDSP_AUDIO_ERR_IO;
        return 0;
    }
    int got = audio_ogg_read(io, out, size * count);
    return got < 0 ? 0 : (size_t)got / size;
}

static int seek_callback(void *context, ogg_int64_t offset, int whence)
{
    return audio_ogg_seek(context, offset, whence);
}

static long tell_callback(void *context)
{
    audio_ogg_io *io = context;
    return (long)opendsp_audio_input_tell(&io->codec->in);
}

static Result vorbis_seek(audio_codec *c, u64 frame)
{
    vorbis_data *v = c->data;
    if (frame > c->info.total_frames)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
#ifdef OPENDSP_CODEC_BANKS
    if (v->dsp && !ogg_backend_context_valid(v->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
#endif
    v->io.error = 0;
    AUDIO_OGG_ENTER(&v->io);
    u64 begin = svcGetSystemTick();
    int ret = ov_pcm_seek(&v->file, (ogg_int64_t)frame);
    if (!ret && ov_pcm_tell(&v->file) != (ogg_int64_t)frame)
        ret = OV_EBADLINK;
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    c->memory = vorbis_memory(v);
    opendsp_audio_ogg_leave();
    if (ret || R_FAILED(v->io.error))
        return audio_ogg_error(&v->io, OPENDSP_AUDIO_ERR_FORMAT);
    c->position = frame;
    return 0;
}

static Result vorbis_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    vorbis_data *v = c->data;
    *frames = 0;
#ifdef OPENDSP_CODEC_BANKS
    if (v->dsp && !ogg_backend_context_valid(v->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
#endif
    if (v->io.memory.failed)
        return OPENDSP_AUDIO_ERR_MEMORY;
    if (!capacity || c->position == c->info.total_frames)
        return 0;
    if (capacity > INT_MAX / (2u * c->info.channels))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    v->io.error = 0;
    AUDIO_OGG_ENTER(&v->io);
    int link = 0;
    u32 requested = capacity > c->info.total_frames - c->position ?
                    c->info.total_frames - c->position : capacity;
    u64 begin = svcGetSystemTick();
    long bytes = ov_read(&v->file, out, requested * c->info.channels * 2u, &link);
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    c->memory = vorbis_memory(v);
    opendsp_audio_ogg_leave();
    if (bytes < 0)
        return audio_ogg_error(&v->io, OPENDSP_AUDIO_ERR_FORMAT);
    if (R_FAILED(v->io.error))
        return v->io.error;
    if (link || bytes % (2u * c->info.channels))
        return OPENDSP_AUDIO_ERR_FORMAT;
    *frames = bytes / (2u * c->info.channels);
    if (*frames > c->info.total_frames - c->position)
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (!*frames && c->position < c->info.total_frames)
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    c->position += *frames;
    c->stats.decoded_frames += *frames;
    return 0;
}

static void vorbis_close(audio_codec *c)
{
    vorbis_data *v = c->data;
    if (!v)
        return;
#ifdef OPENDSP_CODEC_BANKS
    finish_headers(v->dsp, &v->headers_pending, &c->in);
    if (v->dsp) aac_backend_close(v->dsp);
    free(v->headers[0]);
    free(v->headers[1]);
#endif
    opendsp_audio_ogg_enter(&v->io.memory);
    if (!v->io.memory.failed) {
        if (setjmp(v->io.memory.escape) == 0) {
            if (v->opened)
                ov_clear(&v->file);
        }
    }
    opendsp_audio_ogg_release();
    opendsp_audio_ogg_leave();
    free(v);
    c->data = NULL;
}

static const audio_codec_ops vorbis_ops = {vorbis_read, vorbis_seek, vorbis_close, NULL};
#ifdef OPENDSP_CODEC_BANKS
static const audio_codec_ops vorbis_dsp_ops = {vorbis_read, vorbis_seek, vorbis_close, vorbis_stats};
#endif

Result opendsp_audio_vorbis_open(audio_codec *c, const audio_loop_tags *tags, u32 backend,
                                 audio_vorbis_start *start)
{
    vorbis_data *v = calloc(1, sizeof(*v));
    if (!v) {
#ifdef OPENDSP_CODEC_BANKS
        audio_vorbis_start_cancel(start, c);
#endif
        return OPENDSP_AUDIO_ERR_MEMORY;
    }
    c->data = v;
    c->ops = &vorbis_ops;
    v->io.codec = c;
#ifdef OPENDSP_CODEC_BANKS
    // The setup header is already on the DSP.
    v->dsp = start->dsp;
    v->headers[0] = start->headers[0];
    v->headers[1] = start->headers[1];
    v->header_bytes[0] = start->header_bytes[0];
    v->header_bytes[1] = start->header_bytes[1];
    v->headers_pending = start->pending;
    Result started = start->result;
    memset(start, 0, sizeof(*start));
#else
    (void)start;
#endif
    AUDIO_OGG_ENTER(&v->io);
    ov_callbacks callbacks = {read_callback, seek_callback, NULL, tell_callback};
    int ret = ov_open_callbacks(&v->io, &v->file, NULL, 0, callbacks);
    v->opened = ret == 0;
    if (ret || R_FAILED(v->io.error)) {
        opendsp_audio_ogg_leave();
        return audio_ogg_error(&v->io, OPENDSP_AUDIO_ERR_FORMAT);
    }
    vorbis_info *info = ov_info(&v->file, 0);
    ogg_int64_t total = ov_pcm_total(&v->file, -1);
    if (ov_streams(&v->file) != 1 || !info || (info->channels != 1 && info->channels != 2) ||
        info->rate < 1000 || info->rate > 130912 || total < 0) {
        opendsp_audio_ogg_leave();
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    }
    c->info.sample_rate = info->rate;
    c->info.channels = info->channels;
    c->info.bits = 16;
    c->info.format = OPENDSP_AUDIO_VORBIS;
    c->info.duration_known = true;
    c->info.total_frames = (u64)total;
    c->backend = OPENDSP_AAC_BACKEND_ARM11;
    c->seekable = true;
    c->memory = vorbis_memory(v);
    opendsp_audio_ogg_leave();
#ifdef OPENDSP_CODEC_BANKS
    // Console real time limits, measured.
    bool wanted = backend == OPENDSP_AAC_BACKEND_DSP;
    if (backend == OPENDSP_AAC_BACKEND_AUTO && c->info.total_frames && info->rate <= 48000)
        wanted = (u64)c->in.size * 8 * info->rate / c->info.total_frames <= 288000;
    if (backend == OPENDSP_AAC_BACKEND_AUTO && !wanted) {
        finish_headers(v->dsp, &v->headers_pending, &c->in);
        if (v->dsp) aac_backend_close(v->dsp);
        v->dsp = NULL;
        free(v->headers[0]); free(v->headers[1]);
        v->headers[0] = v->headers[1] = NULL;
        v->header_bytes[0] = v->header_bytes[1] = 0;
    }
    if (wanted) {
        Result result = started;
        if (R_SUCCEEDED(result) && !v->dsp) result = OPENDSP_AUDIO_ERR_FORMAT;
        // AUTO still has ARM11 if setup fails.
        if (R_SUCCEEDED(result) && backend == OPENDSP_AAC_BACKEND_AUTO)
            result = finish_headers(v->dsp, &v->headers_pending, &c->in);
        if (R_SUCCEEDED(result)) result = connect_dsp(v);
        if (R_FAILED(result)) {
            finish_headers(v->dsp, &v->headers_pending, &c->in);
            if (v->dsp) aac_backend_close(v->dsp);
            v->dsp = NULL;
            if (backend == OPENDSP_AAC_BACKEND_DSP || R_FAILED(v->io.error) || v->io.memory.failed)
                return audio_ogg_error(&v->io, result);
            free(v->headers[0]); free(v->headers[1]);
            v->headers[0] = v->headers[1] = NULL;
            v->header_bytes[0] = v->header_bytes[1] = 0;
        } else {
            c->backend = OPENDSP_AAC_BACKEND_DSP;
            c->ops = &vorbis_dsp_ops;
        }
    }
#else
    if (backend == OPENDSP_AAC_BACKEND_DSP) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
#endif
    c->memory = vorbis_memory(v);
    audio_loop_tag_finish(tags, c->info.total_frames, &c->loop);
    // ov_open leaves the stream at its first sample.
    Result result = ov_pcm_tell(&v->file) == 0 ? 0 : vorbis_seek(c, 0);
#ifdef OPENDSP_CODEC_BANKS
    // Setup ran during ov_open and seek.
    if (R_SUCCEEDED(result)) result = finish_headers(v->dsp, &v->headers_pending, &c->in);
#endif
    return result;
}
