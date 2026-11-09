// SPDX-License-Identifier: BSD-2-Clause
#include "code.h"
#include "dma.h"
#include "extension.h"

#ifndef OGG_CODE_CHUNK_WORDS
#define OGG_CODE_CHUNK_WORDS 128u
#endif
#ifndef OGG_CODE_STAGING_BYTES
#define OGG_CODE_STAGING_BYTES 320u
#endif
_Static_assert(OGG_CODE_STAGING_BYTES >= 64u && OGG_CODE_CHUNK_WORDS > 0 &&
               OGG_CODE_CHUNK_WORDS <= (OGG_CODE_STAGING_BYTES - 64u) / 2,
               "Code chunk exceeds DMA staging");

#ifdef OGG_CODE_COOPERATIVE
void ogg_code_yield(void);
#endif

int ogg_code_open(ogg_code_loader *loader, u32 arena_begin, u32 arena_end,
                  u32 window_begin, u32 window_end)
{
    if ((arena_begin & 31) || (arena_end & 31) || arena_begin < EXT_FCRAM_START ||
        arena_end > EXT_FCRAM_END || arena_begin >= arena_end ||
        window_begin < 0x100 || window_begin >= window_end || window_end > 0x20000UL)
        return -1;
    loader->arena_begin = arena_begin;
    loader->arena_end = arena_end;
    loader->window_begin = window_begin;
    loader->window_end = window_end;
    loader->source = loader->bytes = loader->loads = loader->loaded_bytes = loader->copied = 0;
    loader->active = loader->pending = 0xffff;
    loader->failed = 0;
    return 0;
}

int ogg_code_begin(ogg_code_loader *loader, u16 bank, u32 source, u32 bytes)
{
    if (!loader->arena_end || loader->failed || bank == 0xffff || !bytes || (bytes & 1) ||
        (source & 31) || source < loader->arena_begin || source >= loader->arena_end ||
        bytes > (loader->window_end - loader->window_begin) * 2 ||
        ((bytes + 31) & ~31UL) > loader->arena_end - source)
        return -1;
    if (loader->source == source && loader->bytes == bytes) {
        if (loader->active == bank) return 0;
        if (loader->pending == bank) return 1;
    }
    loader->active = 0xffff;
    loader->pending = bank;
    loader->source = source;
    loader->bytes = bytes;
    loader->copied = 0;
    return 1;
}

int ogg_code_step(ogg_code_loader *loader)
{
    u32 destination, remaining, page_left;
    u16 count;
    const u16 *staged;
    if (!loader->arena_end || loader->failed) return -1;
    if (loader->pending == 0xffff) return loader->active == 0xffff ? -1 : 0;
    if (loader->copied >= loader->bytes || (loader->copied & 1)) goto failed;
    destination = loader->window_begin + (loader->copied >> 1);
    remaining = (loader->bytes - loader->copied) >> 1;
    page_left = 0x10000UL - (destination & 0xffffUL);
    count = remaining > OGG_CODE_CHUNK_WORDS ? OGG_CODE_CHUNK_WORDS : (u16)remaining;
    if (page_left < count) count = (u16)page_left;
    if (dma_error()) goto failed;
    staged = dma_fetch_words(loader->source + loader->copied, count);
    if (dma_error()) goto failed;
    ogg_program_write((u16)destination, staged, count, (u16)(destination >> 16));
    loader->copied += (u32)count << 1;
    if (loader->copied < loader->bytes) return 1;
    loader->active = loader->pending;
    loader->pending = 0xffff;
    ++loader->loads;
    loader->loaded_bytes += loader->bytes;
    return 0;
failed:
    loader->failed = 1;
    loader->pending = 0xffff;
    return -1;
}

int ogg_code_load(ogg_code_loader *loader, u16 bank, u32 source, u32 bytes)
{
    int pending = ogg_code_begin(loader, bank, source, bytes);
    while (pending > 0) {
        pending = ogg_code_step(loader);
#ifdef OGG_CODE_COOPERATIVE
        if (pending >= 0) ogg_code_yield();
#endif
    }
    return pending;
}
