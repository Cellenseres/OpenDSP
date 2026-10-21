/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdlib.h>
#include <string.h>
#include "aac_backend.h"
#include "mp3_arm11.h"

struct opendspMp3Decoder {
    aac_dsp_backend *dsp;
    mp3_arm11 *arm;
};
Result opendspMp3Open(opendspMp3Decoder **out, const opendspMp3Config *config)
{
    if (!out)
        return OPENDSP_MP3_ERR_INVALID_ARG;
    *out = NULL;
    if (config &&
        (config->struct_size < sizeof(*config) || config->flags ||
         (config->backend != OPENDSP_MP3_BACKEND_AUTO && config->backend != OPENDSP_MP3_BACKEND_DSP &&
          config->backend != OPENDSP_MP3_BACKEND_ARM11)))
        return OPENDSP_MP3_ERR_UNSUPPORTED;
    opendspMp3Decoder *d = calloc(1, sizeof(*d));
    if (!d)
        return OPENDSP_MP3_ERR_NO_MEMORY;
    if (config && config->backend == OPENDSP_MP3_BACKEND_ARM11) {
        d->arm = mp3_arm11_open();
        if (!d->arm) {
            free(d);
            return OPENDSP_MP3_ERR_NO_MEMORY;
        }
        *out = d;
        return 0;
    }
    Result rc = codec_backend_open(&d->dsp, 1);
    if (R_FAILED(rc)) {
        free(d);
        return rc;
    }
    *out = d;
    return 0;
}
Result opendspMp3Decode(opendspMp3Decoder *d, const void *input, size_t bytes, size_t *consumed, s16 *pcm,
                        size_t capacity, size_t *frames, opendspMp3Info *info)
{
    if (!d)
        return OPENDSP_MP3_ERR_INVALID_ARG;
    if (d->arm)
        return mp3_arm11_decode(d->arm, input, bytes, consumed, pcm, capacity, frames, info);
    return mp3_backend_decode(d->dsp, input, bytes, consumed, pcm, capacity, frames, info);
}
void opendspMp3Reset(opendspMp3Decoder *d)
{
    if (d) {
        if (d->arm)
            mp3_arm11_reset(d->arm);
        else
            aac_backend_reset(d->dsp);
    }
}
void opendspMp3Close(opendspMp3Decoder *d)
{
    if (d) {
        if (d->arm)
            mp3_arm11_close(d->arm);
        else
            aac_backend_close(d->dsp);
        free(d);
    }
}
u32 opendspMp3GetBackend(const opendspMp3Decoder *d)
{
    return !d ? OPENDSP_MP3_BACKEND_AUTO : d->arm ? OPENDSP_MP3_BACKEND_ARM11 : OPENDSP_MP3_BACKEND_DSP;
}
Result opendspMp3GetStats(opendspMp3Decoder *d, opendspMp3Stats *stats)
{
    if (!d || !stats || stats->struct_size != sizeof(*stats))
        return OPENDSP_MP3_ERR_INVALID_ARG;
    if (d->arm)
        return OPENDSP_MP3_ERR_BACKEND; /* these are DSP timer/stack counters */
    opendspAacStats common = {.struct_size = sizeof(common)};
    Result rc = aac_backend_stats(d->dsp, &common);
    if (R_FAILED(rc))
        return rc;
    memcpy(stats, &common, sizeof(common));
    stats->struct_size = sizeof(*stats);
    return mp3_backend_stages(d->dsp, stats->stage_ticks);
}
