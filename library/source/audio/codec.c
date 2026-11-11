// SPDX-License-Identifier: BSD-2-Clause
#include <string.h>
#include "codec.h"

bool opendspAudioSupportsBackend(opendspAudioFormat format, u32 backend)
{
    if ((u32)format > OPENDSP_AUDIO_OPUZ ||
        backend > OPENDSP_AAC_BACKEND_DSP)
        return false;
    if (backend == OPENDSP_AAC_BACKEND_AUTO)
        return true;
    if (format == OPENDSP_AUDIO_WAV || format == OPENDSP_AUDIO_MSU1)
        return false;
#ifdef OPENDSP_CODEC_BANKS
    if (format == OPENDSP_AUDIO_OPUS || format == OPENDSP_AUDIO_OPUZ || format == OPENDSP_AUDIO_VORBIS)
        return true;
#endif
    return backend == OPENDSP_AAC_BACKEND_ARM11 ||
           format == OPENDSP_AUDIO_AAC || format == OPENDSP_AUDIO_MP3;
}

Result opendsp_audio_codec_open(audio_codec *c, const char *path, u32 backend, u32 count, u32 frames,
                                u32 flags)
{
    memset(c, 0, sizeof(*c));
    c->info.struct_size = sizeof(c->info);
    c->stats.struct_size = sizeof(c->stats);
    c->loop.struct_size = sizeof(c->loop);
    c->in.stats = &c->stats;
    Result r = opendsp_audio_input_open(&c->in, path);
    if (R_FAILED(r))
        goto fail;
    u8 h[12];
    size_t got;
    r = opendsp_audio_input_read(&c->in, h, sizeof(h), &got);
    if (R_FAILED(r))
        goto fail;
    r = opendsp_audio_input_seek(&c->in, 0);
    if (R_FAILED(r))
        goto fail;
    if (got >= 4 && !memcmp(h, "MSU1", 4))
        r = opendsp_audio_msu1_open(c, false);
    else if (got >= 4 && !memcmp(h, "OPUZ", 4))
        r = opendsp_audio_opuz_open(c, backend);
    else if (flags & OPENDSP_AUDIO_MSU1_PCM)
        r = opendsp_audio_msu1_open(c, true);
    else if (got >= 4 && !memcmp(h, "RIFF", 4))
        r = opendsp_audio_wav_open(c);
    else if (got >= 4 && !memcmp(h, "OggS", 4))
        r = opendsp_audio_ogg_open(c, backend);
    else if (got >= 7 && h[0] == 0xff && (h[1] & 0xf6) == 0xf0)
        r = opendsp_audio_aac_open(c, backend);
    else if ((got >= 3 && !memcmp(h, "ID3", 3)) || (got >= 4 && h[0] == 255 && (h[1] & 0xe6) == 0xe2))
        r = opendsp_audio_mp3_open(c, backend, count ? count : AUDIO_MP3_DSP_BUFFERS, frames ? frames : 1152, flags);
    else
        r = OPENDSP_AUDIO_ERR_FORMAT;
    if (R_SUCCEEDED(r))
        return 0;
fail:
    opendsp_audio_codec_close(c);
    return r;
}

void opendsp_audio_codec_close(audio_codec *c)
{
    if (c->ops)
        c->ops->close(c);
    opendsp_audio_input_close(&c->in);
    c->ops = NULL;
}
