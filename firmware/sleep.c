// SPDX-License-Identifier: BSD-2-Clause
// Optional RAM for effects and decoders.
#include "hw.h"
#include "sleep.h"
#include "extension.h"
#include "dma.h"

extern u16 __sleep_start, __sleep_end;
static u32 arena;

u16 sleep_configure(vu16 *args)
{
    u32 base = ((u32)args[0] << 16) | args[1];
    u32 size = ((u32)args[2] << 16) | args[3];
    if ((base & 127) || base < EXT_FCRAM_START ||
        base > EXT_FCRAM_END - SLEEP_EXT_BYTES || size != SLEEP_EXT_BYTES)
        return EXT_ERR_ADDRESS;
    arena = base;
    return EXT_OK;
}

static u32 cursor;
static u16 restoring;

static void transfer(u16 address, u16 words)
{
    if (restoring) dma_read(cursor, words, address);
    else dma_save(address, words, cursor);
    cursor += (u32)words << 1;
}

static void transfer_state(void)
{
    u16 start = (u16)(unsigned)&__sleep_start;
    u16 end = (u16)(unsigned)&__sleep_end;
    select_bank(0);
    transfer(start, end - start);
    transfer(0x0D40, 0x2C0);
    transfer(0x7280, 0x580);
    transfer(0xC000, 0x4000);
    select_bank(1);
    transfer(0x8000, 0x8000);
    select_bank(0);
}

void sleep_save(void)
{
    vu16 *header = SLEEP_HEADER;
    header[5] = header[6] = 0;
    if (!arena) return;
    restoring = 0;
    cursor = arena;
    transfer_state();
    if (dma_error()) return;
    header[5] = (u16)(arena >> 16);
    header[6] = (u16)arena;
    header[7] = (u16)(cursor - arena);
    header[8] = (u16)((cursor - arena) >> 16);
}

u16 sleep_restore(void)
{
    const vu16 *header = SLEEP_HEADER;
    u32 base = ((u32)header[5] << 16) | header[6];
    u32 size = ((u32)header[8] << 16) | header[7];
    u32 expected = ((u32)((u16)(unsigned)&__sleep_end - (u16)(unsigned)&__sleep_start) +
                    0x2C0UL + 0x580UL + 0x4000UL + 0x8000UL) << 1;
    if (header[2] != SLEEP_MAGIC || header[3] != SLEEP_VERSION ||
        (base & 127) || base < EXT_FCRAM_START || base > EXT_FCRAM_END - SLEEP_EXT_BYTES ||
        size != expected || size > SLEEP_EXT_BYTES) return 0;
    restoring = 1;
    cursor = arena = base;
    transfer_state();
    if (dma_error()) return 0;
    return 1;
}
