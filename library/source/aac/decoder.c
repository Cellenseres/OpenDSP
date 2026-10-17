// SPDX-License-Identifier: BSD-2-Clause
// ADTS framing, resync and per-frame decoding.
#include <string.h>
#include "internal.h"

#define NOISE_SEED 0x2545F491u

typedef struct {
    unsigned protection_absent, object_type, sf_index, channel_config;
    unsigned frame_bytes, blocks, header_bytes;
} adts_header_t;

static int is_sync(const uint8_t *p)
{
    return p[0] == 0xFF && (p[1] & 0xF6) == 0xF0;
}

static int parse_header(const uint8_t *p, adts_header_t *h)
{
    h->protection_absent = p[1] & 1;
    h->object_type = (p[2] >> 6) + 1;
    h->sf_index = (p[2] >> 2) & 15;
    h->channel_config = (unsigned)(p[2] & 1) << 2 | p[3] >> 6;
    h->frame_bytes = (unsigned)(p[3] & 3) << 11 | (unsigned)p[4] << 3 | p[5] >> 5;
    h->blocks = (p[6] & 3) + 1u;
    // CRC: later block positions, then CRC word
    h->header_bytes = 7 + (h->protection_absent ? 0 : 2 * h->blocks);
    return h->sf_index < 13 && h->frame_bytes > h->header_bytes;
}

static void clear_stream_state(aac_decoder_t *d)
{
    memset(d->overlap, 0, sizeof(d->overlap));
    memset(d->prev_shape, 0, sizeof(d->prev_shape));
}

void aac_decoder_reset(aac_decoder_t *d)
{
    clear_stream_state(d);
    d->configured = 0;
    d->synced = 0;
    d->noise_seed = NOISE_SEED;
}

void aac_decoder_init(aac_decoder_t *d)
{
    memset(d, 0, sizeof(*d));
    aac_decoder_reset(d);
}

static void fill_info(const adts_header_t *h, aac_frame_info_t *info, int changed)
{
    if (!info)
        return;
    info->sample_rate = h->sf_index < 12 ? aac_sample_rates[h->sf_index] : h->sf_index == 12 ? 7350 : 0;
    info->channels = (uint8_t)h->channel_config;
    info->blocks = (uint8_t)h->blocks;
    info->changed = (uint8_t)changed;
    info->sbr = 0;
    info->object_type = (uint8_t)h->object_type;
    info->frame_bytes = (uint16_t)h->frame_bytes;
}

static aac_status_t decode_frame(aac_decoder_t *d, const uint8_t *in, const adts_header_t *h, int16_t *pcm)
{
    aac_bits_t b;
    unsigned i;
    b.buf = in;
    b.bytes = h->frame_bytes;
    b.pos = h->header_bytes * 8;
    for (i = 0; i < h->blocks; i++) {
        aac_status_t s = aac_decode_raw_block(d, &b, pcm + i * AAC_FRAME_SAMPLES * d->channels);
        if (s != AAC_OK)
            return s;
        if (!h->protection_absent && h->blocks > 1)
            b.pos += 16;
        if (bits_overrun(&b))
            return AAC_CORRUPT;
    }
    return AAC_OK;
}

aac_status_t aac_decode(aac_decoder_t *d, const uint8_t *in, size_t in_bytes, size_t *consumed,
                        int16_t *pcm, size_t pcm_frames, size_t *frames, aac_frame_info_t *info)
{
    adts_header_t h;
    size_t i = 0, needed;
    int changed;
    aac_status_t s;
    if (consumed)
        *consumed = 0;
    if (frames)
        *frames = 0;
    if (!d || !consumed || !frames || !pcm || (!in && in_bytes))
        return AAC_INVALID_ARGUMENT;
    if (!in_bytes)
        return AAC_NEED_MORE_DATA;

    while (i + 1 < in_bytes && !is_sync(in + i))
        i++;
    if (i + 1 >= in_bytes) {
        // a trailing 0xFF may start a sync word
        i = in[in_bytes - 1] == 0xFF ? in_bytes - 1 : in_bytes;
    }
    if (i) {
        *consumed = i;
        d->synced = 0;
        return AAC_RESYNC;
    }
    if (in_bytes < 7)
        return AAC_NEED_MORE_DATA;
    if (!parse_header(in, &h)) {
        *consumed = 1;
        d->synced = 0;
        return AAC_RESYNC;
    }
    if (in_bytes < h.frame_bytes)
        return AAC_NEED_MORE_DATA;
    // unlocked: next frame must start with sync
    if (!d->synced && in_bytes >= h.frame_bytes + 2 && !is_sync(in + h.frame_bytes)) {
        *consumed = 1;
        return AAC_RESYNC;
    }
    changed = !d->configured || d->sf_index != h.sf_index || d->channels != h.channel_config;
    fill_info(&h, info, changed);
    if (h.object_type != 2 || h.sf_index > 11 || h.channel_config < 1 || h.channel_config > 2) {
        *consumed = h.frame_bytes;
        d->synced = 1;
        return AAC_UNSUPPORTED;
    }
    needed = (size_t)h.blocks * AAC_FRAME_SAMPLES;
    if (pcm_frames < needed)
        return AAC_OUTPUT_TOO_SMALL;
    if (changed) {
        clear_stream_state(d);
        d->sf_index = (uint8_t)h.sf_index;
        d->channels = (uint8_t)h.channel_config;
        d->configured = 1;
    }
    *consumed = h.frame_bytes;
    d->sbr_seen = 0;
    s = decode_frame(d, in, &h, pcm);
    if (info)
        info->sbr = d->sbr_seen;
    if (s != AAC_OK) {
        clear_stream_state(d);
        d->synced = 0;
        return s;
    }
    d->synced = 1;
    *frames = needed;
    return AAC_OK;
}
