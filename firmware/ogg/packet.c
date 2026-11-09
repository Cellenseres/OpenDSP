// SPDX-License-Identifier: BSD-2-Clause
#include "packet.h"
#include "dma.h"
#include "extension.h"

#ifdef OGG_INPUT_COOPERATIVE
void ogg_input_yield(void);
#endif

int ogg_input_open(ogg_input *input, u32 address, u32 bytes, u32 arena_begin, u32 arena_end)
{
    input->cache_bytes = 0;
    input->failed = 1;
    if ((arena_begin & 31) || (arena_end & 31) || arena_begin < EXT_FCRAM_START ||
        arena_end > EXT_FCRAM_END || arena_begin >= arena_end || address < arena_begin ||
        address > arena_end || bytes > arena_end - address)
        return -1;
    input->address = address;
    input->bytes = bytes;
    input->arena_begin = arena_begin;
    input->arena_end = arena_end;
    input->failed = 0;
    return 0;
}

int ogg_input_byte(void *context, long offset)
{
    ogg_input *input = context;
    u32 address;
    u16 index;
    if (input->failed || offset < 0 || (unsigned long)offset >= input->bytes)
        return -1;
    address = input->address + (u32)offset;
    if (!input->cache_bytes || address < input->cache_address ||
        address - input->cache_address >= input->cache_bytes) {
        const u16 *source;
        u32 block = address & ~127UL;
        u32 available;
#ifdef OGG_INPUT_COOPERATIVE
        ogg_input_yield();
#endif
        if (block < input->arena_begin)
            block = input->arena_begin;
        available = input->arena_end - block;
        input->cache_bytes = available < 128 ? (u16)available : 128;
        input->cache_address = block;
        if (dma_error()) {
            input->failed = 1;
            return -1;
        }
        source = dma_fetch_words(block, input->cache_bytes / 2);
        if (dma_error()) {
            input->failed = 1;
            return -1;
        }
        for (index = 0; index < input->cache_bytes / 2; ++index)
            input->cache[index] = source[index];
    }
    index = (u16)(address - input->cache_address);
    return (input->cache[index / 2] >> ((index & 1) * 8)) & 255;
}
