// SPDX-License-Identifier: BSD-2-Clause
// Public AAC API over the portable core.
#include <stdlib.h>
#include "opendsp/aac.h"
#include "aac/aac.h"

struct opendspAacDecoder {
    aac_decoder_t core;
    u32 backend;
};

Result opendspAacOpen(opendspAacDecoder **out, const opendspAacConfig *config)
{
    opendspAacDecoder *dec;
    u32 backend = OPENDSP_AAC_BACKEND_AUTO;
    if (!out)
        return OPENDSP_AAC_ERR_INVALID_ARG;
    *out = NULL;
    if (config) {
        if (config->struct_size < sizeof(*config) || config->flags)
            return OPENDSP_AAC_ERR_INVALID_ARG;
        backend = config->backend;
    }
    if (backend == OPENDSP_AAC_BACKEND_DSP)
        return OPENDSP_AAC_ERR_BACKEND;
    if (backend != OPENDSP_AAC_BACKEND_AUTO && backend != OPENDSP_AAC_BACKEND_ARM11)
        return OPENDSP_AAC_ERR_INVALID_ARG;
    dec = malloc(sizeof(*dec));
    if (!dec)
        return OPENDSP_AAC_ERR_NO_MEMORY;
    aac_decoder_init(&dec->core);
    dec->backend = OPENDSP_AAC_BACKEND_ARM11;
    *out = dec;
    return 0;
}

static Result to_result(aac_status_t s)
{
    switch (s) {
    case AAC_OK:
        return 0;
    case AAC_RESYNC:
        return OPENDSP_AAC_INFO_SKIPPED;
    case AAC_NEED_MORE_DATA:
        return OPENDSP_AAC_ERR_NEED_MORE_DATA;
    case AAC_CORRUPT:
        return OPENDSP_AAC_ERR_CORRUPT;
    case AAC_UNSUPPORTED:
        return OPENDSP_AAC_ERR_UNSUPPORTED;
    case AAC_OUTPUT_TOO_SMALL:
        return OPENDSP_AAC_ERR_OUTPUT_TOO_SMALL;
    default:
        return OPENDSP_AAC_ERR_INVALID_ARG;
    }
}

Result opendspAacDecode(opendspAacDecoder *dec, const void *input, size_t input_bytes, size_t *bytes_consumed,
                        s16 *pcm_interleaved, size_t pcm_capacity_frames, size_t *frames_written,
                        opendspAacInfo *info)
{
    aac_frame_info_t frame = {0};
    aac_status_t s;
    if (!dec)
        return OPENDSP_AAC_ERR_INVALID_ARG;
    s = aac_decode(&dec->core, input, input_bytes, bytes_consumed, pcm_interleaved, pcm_capacity_frames,
                   frames_written, &frame);
    if (info && frame.frame_bytes) {
        info->sample_rate = frame.sample_rate;
        info->channels = frame.channels;
        info->frame_samples = AAC_FRAME_SAMPLES;
        info->blocks = frame.blocks;
        info->object_type = frame.object_type;
        info->flags = (frame.changed ? OPENDSP_AAC_INFO_CHANGED : 0) | (frame.sbr ? OPENDSP_AAC_INFO_SBR_IGNORED : 0);
    }
    return to_result(s);
}

void opendspAacReset(opendspAacDecoder *dec)
{
    if (dec)
        aac_decoder_reset(&dec->core);
}

void opendspAacClose(opendspAacDecoder *dec)
{
    free(dec);
}
