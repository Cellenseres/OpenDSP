/* SPDX-License-Identifier: BSD-2-Clause */
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <opusfile.h>
#include "ogg.h"
#include "../internal.h"
#ifdef OPENDSP_CODEC_BANKS
#include "../ogg_backend.h"
#endif

typedef struct {
    audio_ogg_io io;
    OggOpusFile *file;
#ifdef OPENDSP_CODEC_BANKS
    aac_dsp_backend *dsp;
    u32 decode_generation;
    bool dsp_faulted;
#endif
} opus_data;

static u32 opus_memory(const opus_data *o)
{
    u32 bytes = sizeof(*o) + o->io.memory.peak;
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp) bytes += aac_backend_memory(o->dsp);
#endif
    return bytes;
}

#ifdef OPENDSP_CODEC_BANKS
static int decode_callback(void *context, OpusMSDecoder *decoder, void *pcm, const ogg_packet *packet,
                            int frames, int channels, int format, int link)
{
    opus_data *o = context;
    audio_input *input = &o->io.codec->in;
    Result result;
    opus_int32 gain = 0;
    const char *unsupported = NULL;
    if (format != OP_DEC_FORMAT_SHORT) unsupported = "Opus callback sample format";
    else if (link) unsupported = "Opus callback chained stream";
    else if (channels != o->io.codec->info.channels) unsupported = "Opus callback channel count";
    else if (frames <= 0 || frames > 5760) unsupported = "Opus callback frame count";
    else if (packet->bytes < 0 || packet->bytes >= 65536) unsupported = "Opus callback packet size";
    else if (opus_multistream_decoder_ctl(decoder, OPUS_GET_GAIN(&gain))) unsupported = "Opus callback gain query";
    if (unsupported) {
        o->io.error = OPENDSP_AUDIO_ERR_UNSUPPORTED;
        opendsp_codec_error(unsupported, o->io.error);
        return OPUS_INTERNAL_ERROR;
    }
    u32 generation = op_decode_generation(o->file);
    if (o->dsp_faulted || generation != o->decode_generation) {
        result = ogg_backend_reset(o->dsp, input->generation, input->expected);
        if (R_FAILED(result)) goto failed;
        o->decode_generation = generation;
        o->dsp_faulted = false;
    }
    result = ogg_backend_decode(o->dsp, packet->packet, packet->bytes, pcm, (u16)frames, (s16)gain,
                                input->generation, input->expected);
    if (R_SUCCEEDED(result)) return 0;
failed:
    o->dsp_faulted = true;
    o->io.error = result;
    return OPUS_INTERNAL_ERROR;
}

static Result opus_stats(audio_codec *c, opendspAudioDecoderStats *stats)
{
    opus_data *o = c->data;
    if (!o->dsp) return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (!ogg_backend_context_valid(o->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
    opendspAacStats native = {.struct_size = sizeof(native)};
    Result result = aac_backend_stats(o->dsp, &native);
    if (R_FAILED(result)) return result;
    memset(stats, 0, sizeof(*stats));
    memcpy(stats, &native, sizeof(native));
    stats->struct_size = sizeof(*stats);
    return 0;
}
#endif

#ifdef OPENDSP_CODEC_BANKS
// Console real time limits, measured.
#define OPUS_AUTO_BITRATE 288000
#define OPUS_AUTO_SHORT_BITRATE 128000

static bool auto_uses_dsp(const audio_codec *c, const opus_data *o)
{
    if (!c->first_toc_known) return false;
    int per_frame = opus_packet_get_samples_per_frame(&c->first_toc, 48000);
    opus_int32 bitrate = op_bitrate(o->file, -1);
    if (per_frame < 480 || bitrate <= 0) return false;
    return bitrate <= (per_frame < 960 ? OPUS_AUTO_SHORT_BITRATE : OPUS_AUTO_BITRATE);
}
#endif

static int read_callback(void *context, unsigned char *out, int bytes)
{
    return audio_ogg_read(context, out, bytes);
}

static int seek_callback(void *context, opus_int64 offset, int whence)
{
    return audio_ogg_seek(context, offset, whence);
}

static opus_int64 tell_callback(void *context)
{
    audio_ogg_io *io = context;
    return (opus_int64)opendsp_audio_input_tell(&io->codec->in);
}

static Result opus_seek(audio_codec *c, u64 frame)
{
    opus_data *o = c->data;
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp && !ogg_backend_context_valid(o->dsp)) {
        if (!ogg_backend_recoverable(o->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
        o->dsp_faulted = true;
    }
#endif
    if (o->io.memory.failed)
        return OPENDSP_AUDIO_ERR_MEMORY;
    if (frame > c->info.total_frames)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    /* EOF has no remaining audio. */
    if (frame == c->info.total_frames) {
        c->position = frame;
        return 0;
    }
    o->io.error = 0;
    AUDIO_OGG_ENTER(&o->io);
    u64 begin = svcGetSystemTick();
    int ret = 0;
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp_faulted) ret = op_raw_seek(o->file, 0);
#endif
    if (!ret) ret = op_pcm_seek(o->file, (ogg_int64_t)frame);
    if (!ret && op_pcm_tell(o->file) != (ogg_int64_t)frame)
        ret = OP_EBADLINK;
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    c->memory = opus_memory(o);
    opendsp_audio_ogg_leave();
    if (ret || R_FAILED(o->io.error))
        return audio_ogg_error(&o->io, OPENDSP_AUDIO_ERR_FORMAT);
    c->position = frame;
    return 0;
}

static Result opus_read(audio_codec *c, void *out, u32 capacity, u32 *frames)
{
    opus_data *o = c->data;
    *frames = 0;
#ifdef OPENDSP_CODEC_BANKS
    if (o->dsp && !ogg_backend_context_valid(o->dsp)) {
        if (!ogg_backend_recoverable(o->dsp)) return OPENDSP_AUDIO_ERR_CONTEXT;
        o->dsp_faulted = true;
    }
#endif
    if (o->io.memory.failed)
        return OPENDSP_AUDIO_ERR_MEMORY;
    if (!capacity || c->position == c->info.total_frames)
        return 0;
    if (capacity > INT_MAX / c->info.channels)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    o->io.error = 0;
    AUDIO_OGG_ENTER(&o->io);
    int link = 0;
    u64 begin = svcGetSystemTick();
    int count = op_read(o->file, out, capacity * c->info.channels, &link);
    c->stats.decode_ticks += svcGetSystemTick() - begin;
    c->memory = opus_memory(o);
    opendsp_audio_ogg_leave();
    if (count < 0)
        return audio_ogg_error(&o->io, OPENDSP_AUDIO_ERR_FORMAT);
    if (R_FAILED(o->io.error))
        return o->io.error;
    if (link || (u64)count > c->info.total_frames - c->position)
        return OPENDSP_AUDIO_ERR_FORMAT;
    if (!count && c->position < c->info.total_frames)
        return OPENDSP_AUDIO_ERR_TRUNCATED;
    c->position += count;
    c->stats.decoded_frames += count;
    *frames = count;
    return 0;
}

static void opus_close(audio_codec *c)
{
    opus_data *o = c->data;
    if (!o)
        return;
#ifdef OPENDSP_CODEC_BANKS
    ogg_backend_park(o->dsp);
#endif
    opendsp_audio_ogg_enter(&o->io.memory);
    if (!o->io.memory.failed) {
        if (setjmp(o->io.memory.escape) == 0)
            op_free(o->file);
    }
    opendsp_audio_ogg_release();
    opendsp_audio_ogg_leave();
    free(o);
    c->data = NULL;
}

static const audio_codec_ops opus_ops = {opus_read, opus_seek, opus_close,
#ifdef OPENDSP_CODEC_BANKS
    opus_stats
#else
    NULL
#endif
};

Result opendsp_audio_opus_open(audio_codec *c, const audio_loop_tags *tags, u32 backend)
{
    opus_data *o = calloc(1, sizeof(*o));
    if (!o)
        return OPENDSP_AUDIO_ERR_MEMORY;
    c->data = o;
    c->ops = &opus_ops;
    o->io.codec = c;
    AUDIO_OGG_ENTER(&o->io);
    OpusFileCallbacks callbacks = {read_callback, seek_callback, tell_callback, NULL};
    int error;
    o->file = op_open_callbacks(&o->io, &callbacks, NULL, 0, &error);
    if (!o->file || R_FAILED(o->io.error)) {
        opendsp_audio_ogg_leave();
        return audio_ogg_error(&o->io, error == OP_EFAULT ? OPENDSP_AUDIO_ERR_MEMORY :
                               OPENDSP_AUDIO_ERR_FORMAT);
    }
    const OpusHead *head = op_head(o->file, 0);
    ogg_int64_t total = op_pcm_total(o->file, -1);
    if (op_link_count(o->file) != 1 || !head || head->mapping_family != 0 ||
        (head->channel_count != 1 && head->channel_count != 2) || total < 0) {
        opendsp_audio_ogg_leave();
        opendsp_codec_error("Opus stream layout", OPENDSP_AUDIO_ERR_UNSUPPORTED);
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    }
    c->info.sample_rate = 48000;
    c->info.channels = head->channel_count;
    c->info.bits = 16;
    c->info.format = OPENDSP_AUDIO_OPUS;
    c->info.duration_known = true;
    c->info.total_frames = (u64)total;
    c->backend = OPENDSP_AAC_BACKEND_ARM11;
    c->seekable = true;
    c->memory = opus_memory(o);
    opendsp_audio_ogg_leave();
#ifdef OPENDSP_CODEC_BANKS
    if (backend == OPENDSP_AAC_BACKEND_DSP || (backend == OPENDSP_AAC_BACKEND_AUTO && auto_uses_dsp(c, o))) {
        Result result = ogg_backend_reuse(&o->dsp, c->info.channels);
        // AUTO keeps the ARM11 decoder.
        if (R_FAILED(result) && backend == OPENDSP_AAC_BACKEND_DSP) return result;
        if (R_FAILED(result)) o->dsp = NULL;
        if (o->dsp) {
            c->backend = OPENDSP_AAC_BACKEND_DSP;
            o->decode_generation = op_decode_generation(o->file);
            op_set_decode_callback(o->file, decode_callback, o);
            c->memory = opus_memory(o);
        }
    }
#else
    if (backend == OPENDSP_AAC_BACKEND_DSP) {
        opendsp_codec_error("Opus DSP missing from build", OPENDSP_AUDIO_ERR_UNSUPPORTED);
        return OPENDSP_AUDIO_ERR_UNSUPPORTED;
    }
#endif
    audio_loop_tag_finish(tags, c->info.total_frames, &c->loop);
    return opus_seek(c, 0);
}
