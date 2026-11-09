// SPDX-License-Identifier: BSD-2-Clause
#include "output.h"
#include "dma.h"
#include "extension.h"

int ogg_pcm_open(ogg_pcm_output *output, u32 begin, u32 end, u16 slots, u16 frames, u16 channels)
{
    u32 stride, needed = 0;
    u16 index;
    if (!slots || slots > OGG_PCM_SLOTS || !frames || frames > 8192 ||
        channels < 1 || channels > 2 || (begin & 127) || (end & 127) ||
        begin < EXT_FCRAM_START || end > EXT_FCRAM_END || begin >= end)
        return -1;
    stride = (u32)frames << (channels == 2 ? 2 : 1);
    stride = (stride + 127) & ~127UL;
    for (index = 0; index < slots; ++index)
        needed += stride;
    if (needed > end - begin)
        return -1;
    output->slots = slots;
    output->frames = frames;
    output->channels = channels;
    output->index = output->used = output->produced = output->consumed = 0;
    output->failed = output->finished = 0;
    output->ready_frames = 0;
    for (index = 0; index < slots; ++index) {
        output->address[index] = begin;
        output->lengths[index] = 0;
        begin += stride;
    }
    return 0;
}

static void publish(ogg_pcm_output *output)
{
    output->lengths[output->index] = output->used;
    output->ready_frames += output->used;
    ++output->produced;
    output->used = 0;
    if (++output->index == output->slots)
        output->index = 0;
}

static int write_pcm(ogg_pcm_output *output, const u16 *source, u16 words, u32 address)
{
    while (words) {
        u16 lead = (u16)((address & 31) >> 1), count, index;
        if (dma_error())
            return -1;
        if (lead || words < 16) {
            const u16 *staged = dma_fetch_words(address & ~31UL, 16);
            if (dma_error())
                return -1;
            for (index = 0; index < 16; ++index)
                output->bounce[index] = staged[index];
            count = 16 - lead;
            if (count > words) count = words;
            for (index = 0; index < count; ++index)
                output->bounce[lead + index] = source[index];
            dma_write_buffer(output->bounce, 16, address & ~31UL);
        } else {
            count = words > 128 ? 128 : words;
            count &= (u16)~15u;
            for (index = 0; index < count; ++index)
                output->bounce[index] = source[index];
            dma_write_buffer(output->bounce, count, address);
        }
        if (dma_error())
            return -1;
        source += count;
        words -= count;
        address += (u32)count << 1;
    }
    return 0;
}

int ogg_pcm_write(ogg_pcm_output *output, const s16 *pcm, u16 frames)
{
    u16 count, words;
    u32 offset;
    if (!output->slots || output->failed || output->finished || (!pcm && frames))
        return -1;
    if ((u16)(output->produced - output->consumed) == output->slots || !frames)
        return 0;
    count = output->frames - output->used;
    if (count > frames) count = frames;
    if (count > 128) count = 128;
    words = output->channels == 2 ? (u16)(count << 1) : count;
    offset = (u32)output->used << (output->channels == 2 ? 2 : 1);
    if (write_pcm(output, (const u16 *)pcm, words, output->address[output->index] + offset)) {
        output->failed = 1;
        return -1;
    }
    output->used += count;
    if (output->used == output->frames)
        publish(output);
    return count;
}

int ogg_pcm_finish(ogg_pcm_output *output)
{
    if (!output->slots || output->failed)
        return -1;
    if (!output->finished && output->used)
        publish(output);
    output->finished = 1;
    return 0;
}

int ogg_pcm_ack(ogg_pcm_output *output, u16 consumed)
{
    if (!output->slots || (u16)(consumed - output->consumed) > (u16)(output->produced - output->consumed))
        return -1;
    output->consumed = consumed;
    return 0;
}

int ogg_pcm_slot(const ogg_pcm_output *output, u16 sequence, u32 *address, u16 *frames)
{
    u16 index;
    if (!output->slots || (u16)(sequence - output->consumed) >= (u16)(output->produced - output->consumed))
        return -1;
    index = output->index + output->slots - (u16)(output->produced - sequence);
    if (index >= output->slots)
        index -= output->slots;
    *address = output->address[index];
    *frames = output->lengths[index];
    return 0;
}
