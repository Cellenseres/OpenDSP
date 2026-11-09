// SPDX-License-Identifier: BSD-2-Clause
#include "opus_packet.h"

void ogg_opus_poll(void);

static s16 read_byte(ogg_opus_packet *packet, u16 *position, u16 end)
{
    s16 value;
    if (*position >= end) return -1;
    value = ogg_input_byte(&packet->input, (long)*position);
    if (value >= 0) ++*position;
    return value;
}

static s16 read_size(ogg_opus_packet *packet, u16 *position, u16 end)
{
    s16 first = read_byte(packet, position, end), second;
    if (first < 252) return first;
    second = read_byte(packet, position, end);
    return second < 0 ? -1 : first + 4 * second;
}

int ogg_opus_packet_open(ogg_opus_packet *packet, u32 address, u32 bytes, u32 arena_begin, u32 arena_end)
{
    u16 position = 0, end = (u16)bytes, sum = 0;
    u16 count = 1, samples, index, variable = 0;
    s16 toc, flags = 0;
    if (!packet) return -1;
    packet->failed = 1;
    packet->frames = packet->samples = 0;
    if (!bytes || bytes > 65535 || ogg_input_open(&packet->input, address, bytes, arena_begin, arena_end))
        return -1;
    toc = read_byte(packet, &position, end);
    if (toc < 0) return -1;
    if (toc & 128) samples = (u16)(120u << ((toc >> 3) & 3));
    else if ((toc & 96) == 96) samples = toc & 8 ? 960 : 480;
    else {
        u16 duration = (toc >> 3) & 3;
        samples = duration == 3 ? 2880 : (u16)(480u << duration);
    }
    if ((toc & 3) == 1) count = 2;
    else if ((toc & 3) == 2) {
        count = 2;
        variable = 1;
    } else if ((toc & 3) == 3) {
        flags = read_byte(packet, &position, end);
        if (flags < 0) return -1;
        count = flags & 63;
        variable = flags & 128;
        if (flags & 64) {
            s16 padding;
            do {
                u16 skipped;
                padding = read_byte(packet, &position, end);
                if (padding < 0) return -1;
                skipped = padding == 255 ? 254 : (u16)padding;
                if (skipped > (u16)(end - position)) return -1;
                end -= skipped;
            } while (padding == 255);
        }
    }
    if (!count || count > 48 || (u32)samples * count > 5760) return -1;
    if (variable) {
        for (index = 0; index + 1 < count; ++index) {
            s16 size = read_size(packet, &position, end);
            if (size < 0 || (u32)size > (u16)(end - position)) return -1;
            packet->bytes[index] = (u16)size;
            sum += (u16)size;
        }
        if (sum > (u16)(end - position) || (u16)(end - position) - sum > 1275) return -1;
        packet->bytes[count - 1] = (u16)((u16)(end - position) - sum);
    } else {
        u16 size = (u16)(end - position) / count;
        if (size > 1275 || size * count != (u16)(end - position)) return -1;
        for (index = 0; index < count; ++index) packet->bytes[index] = (u16)size;
    }
    for (index = 0; index < count; ++index) {
        packet->offset[index] = (u16)position;
        position += packet->bytes[index];
    }
    if (position != end) return -1;
    packet->frames = count;
    packet->samples = samples;
    packet->toc = (u16)toc;
    packet->failed = 0;
    return 0;
}

int ogg_opus_packet_frame(ogg_opus_packet *packet, u16 index, unsigned char *output, u16 capacity)
{
    u16 position, left;
    if (!packet || !output) return -1;
    if (packet->failed || index >= packet->frames || capacity <= packet->bytes[index]) {
        packet->failed = 1;
        return -1;
    }
    output[0] = packet->toc & 252;
    position = packet->offset[index];
    left = packet->bytes[index];
    output++;
    while (left) {
        ogg_input *input = &packet->input;
        const u16 *words;
        u16 cached, odd, pairs;
        s16 value;
        // Large packets copy many blocks.
        ogg_opus_poll();
        value = ogg_input_byte(input, (long)position);
        if (value < 0) {
            packet->failed = 1;
            return -1;
        }
        *output++ = (unsigned char)value;
        ++position;
        --left;
        // Rest of the cached block
        cached = (u16)(input->address + position - input->cache_address);
        odd = input->cache_bytes - cached;
        if (odd > left) odd = left;
        left -= odd;
        position += odd;
        words = input->cache + cached / 2;
        if (cached & 1 && odd) {
            *output++ = *words++ >> 8;
            --odd;
        }
        for (pairs = odd >> 1; pairs; --pairs) {
            u16 word = *words++;
            output[0] = word & 255;
            output[1] = word >> 8;
            output += 2;
        }
        if (odd & 1) *output++ = *words & 255;
    }
    return packet->bytes[index] + 1;
}
