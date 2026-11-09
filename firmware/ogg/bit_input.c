// SPDX-License-Identifier: BSD-2-Clause
#include "bit_input.h"
#include "dma.h"

#ifdef __teak__
u32 ogg_cached_bits(const u16 *words, u16 shift, u16 bits);
#endif

#ifdef __teak__
__attribute__((section(".vorbis_shared")))
#endif
u32 ogg_input_bits(ogg_input *input, long offset, u16 bit, u16 bits)
{
    const u16 *source;
    u16 index, shift, needed, words;
    u32 address;
    if (input->failed || offset < 0 || (unsigned long)offset > input->bytes || bit > 7 || bits > 32)
        goto failed;
    if (!bits) return 0;
    needed = (bit + bits + 7) / 8;
    if (needed > input->bytes - (u32)offset) goto failed;
    address = input->address + (u32)offset;
    if (!input->cache_bytes || address - input->cache_address >= input->cache_bytes)
        if (ogg_input_byte(input, offset) < 0) goto failed;
    index = (u16)((u16)address - (u16)input->cache_address);
    shift = (index & 1) * 8 + bit;
    index /= 2;
    words = (bits + shift + 15) / 16;
    source = input->cache + index;
    if (index + words > input->cache_bytes / 2) {
        // Window caches are 32 byte aligned.
        if (ogg_input_window(input, offset) < 0) goto failed;
        index = (u16)((u16)address - (u16)input->cache_address);
        shift = (index & 1) * 8 + bit;
        index /= 2;
        words = (bits + shift + 15) / 16;
        if (index + words > input->cache_bytes / 2) goto failed;
        source = input->cache + index;
    }
#ifdef __teak__
    return ogg_cached_bits(source, shift, bits);
#else
    u32 result = (u32)source[0] >> shift;
    if (bits + shift > 16) result |= (u32)source[1] << (16 - shift);
    if (bits + shift > 32) result |= (u32)source[2] << (32 - shift);
    return result & ((u32)~(u32)0 >> (32 - bits));
#endif
failed:
    input->failed = 1;
    return (u32)~(u32)0;
}

// Keeps 32 bytes from offset in the cache.
#ifdef __teak__
__attribute__((section(".vorbis_shared")))
#endif
int ogg_input_window(ogg_input *input, long offset)
{
    const u16 *source;
    u32 address, block, available;
    u16 index, bytes;
    if (input->failed || offset < 0 || (unsigned long)offset >= input->bytes) return -1;
    address = input->address + (u32)offset;
    block = address & ~31UL;
    if (block < input->arena_begin) block = input->arena_begin;
    if (input->cache_bytes && address >= input->cache_address &&
        (address - input->cache_address + 32 <= input->cache_bytes ||
         (block == input->cache_address && address - block < input->cache_bytes)))
        return 0;
    available = input->arena_end - block;
    bytes = available < 128 ? (u16)available : 128;
    if (dma_error()) goto failed;
    source = dma_fetch_words(block, bytes / 2);
    if (dma_error()) goto failed;
    for (index = 0; index < bytes / 2; ++index) input->cache[index] = source[index];
    input->cache_address = block;
    input->cache_bytes = bytes;
    return 0;
failed:
    input->failed = 1;
    return -1;
}
