// SPDX-License-Identifier: BSD-2-Clause
#include <string.h>
#include "codec.h"

Result opendsp_audio_codec_open(audio_codec *c, const char *path, u32 backend)
{
    memset(c, 0, sizeof(*c));
    c->info.struct_size = sizeof(c->info);
    c->stats.struct_size = sizeof(c->stats);
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
    if (got >= 4 && !memcmp(h, "RIFF", 4))
        r = opendsp_audio_wav_open(c);
    else if (got >= 7 && h[0] == 0xff && (h[1] & 0xf6) == 0xf0)
        r = opendsp_audio_aac_open(c, backend);
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
    if (c->in.file)
        fclose(c->in.file);
    c->in.file = NULL;
    c->ops = NULL;
}
