// SPDX-License-Identifier: BSD-2-Clause
#include "banks.h"
#include "code.h"
#include "dma.h"
#include "extension.h"
#include "codec_banks_protocol.h"

#ifndef CODEC_PROGRAM_WINDOW
#error CODEC_PROGRAM_WINDOW is required
#endif

int codec_code_open(ogg_code_loader *loader, u32 begin, u32 end, u32 first, u32 last);
int codec_code_begin(ogg_code_loader *loader, u16 bank, u32 source, u32 bytes);
int codec_code_step(ogg_code_loader *loader);

static ogg_code_loader loader;
static u32 directory;
static u16 pool_token, bank_count;
extern const u32 codec_bank_identity[2];

static u32 read_u32(const volatile u16 *words)
{
    return ((u32)words[0] << 16) | words[1];
}

static u16 configure(vu16 *args)
{
    u32 begin = read_u32(args + 2), end = read_u32(args + 4), table = read_u32(args + 6);
    u16 count = args[8];
    u32 table_bytes = ((u32)count * CODEC_BANK_DESCRIPTOR_BYTES + 31) & ~31UL;
    if (args[0] != CODEC_BANK_ABI || !args[1] || !count || count > CODEC_BANK_LIMIT ||
        read_u32(args + 9) != CODEC_PROGRAM_WINDOW)
        return EXT_ERR_ARGUMENT;
    if (read_u32(args + 11) != codec_bank_identity[0] || read_u32(args + 13) != codec_bank_identity[1])
        return EXT_ERR_UNSUPPORTED;
    if ((table & 31) || table < begin || table >= end || table_bytes > end - table)
        return EXT_ERR_ADDRESS;
    if (pool_token) {
        if (pool_token == args[1] && loader.arena_begin == begin && loader.arena_end == end &&
            directory == table && bank_count == count)
            return EXT_OK;
        return EXT_ERR_ARGUMENT;
    }
    if (codec_code_open(&loader, begin, end, CODEC_PROGRAM_WINDOW, 0x20000UL))
        return EXT_ERR_ADDRESS;
    directory = table;
    bank_count = count;
    pool_token = args[1];
    return EXT_OK;
}

static u16 select_bank_code(u16 bank)
{
    const u16 *descriptor;
    u32 source, bytes;
    int pending;
    if (bank >= bank_count) return EXT_ERR_ARGUMENT;
    if (loader.failed || dma_error()) goto failed;
    descriptor = dma_fetch_words(directory + (u32)bank * CODEC_BANK_DESCRIPTOR_BYTES, 4);
    if (dma_error()) goto failed;
    source = read_u32(descriptor);
    bytes = read_u32(descriptor + 2);
    pending = codec_code_begin(&loader, bank, source, bytes);
    return pending < 0 ? EXT_ERR_ADDRESS : EXT_OK;
failed:
    loader.active = loader.pending = 0xffff;
    loader.failed = 1;
    return EXT_ERR_ADDRESS;
}

u16 codec_bank_command(u16 command, vu16 *args, vu16 *reply, u16 busy)
{
    if (command == CODEC_BANK_STATUS) {
        if (!pool_token || args[0] != pool_token) return EXT_ERR_ARGUMENT;
        reply[0] = loader.active;
        reply[1] = loader.pending;
        reply[2] = loader.failed;
        reply[3] = (u16)(loader.copied >> 16);
        reply[4] = (u16)loader.copied;
        return EXT_OK;
    }
    if (busy) return EXT_ERR_ARGUMENT;
    if (command == CODEC_BANK_CONFIGURE) return configure(args);
    if (command == CODEC_BANK_CLOSE && !pool_token) return EXT_OK;
    if (!pool_token || args[0] != pool_token) return EXT_ERR_ARGUMENT;
    if (command == CODEC_BANK_SELECT) return select_bank_code(args[1]);
    if (command == CODEC_BANK_CLOSE) {
        if (dma_error()) {
            dma_quiesce();
            if (dma_error()) return EXT_ERR_ADDRESS;
        }
        pool_token = bank_count = 0;
        directory = loader.arena_end = 0;
        loader.active = loader.pending = 0xffff;
        return EXT_OK;
    }
    return EXT_ERR_COMMAND;
}

int codec_bank_poll(void)
{
    if (!pool_token || loader.pending == 0xffff) return 0;
    return codec_code_step(&loader);
}

u16 codec_bank_ready(u16 bank)
{
    return pool_token && !loader.failed && loader.pending == 0xffff && loader.active == bank;
}

// Program RAM is lost in sleep.
void codec_bank_resume(void)
{
    u16 bank = loader.pending != 0xffff ? loader.pending : loader.active;
    loader.active = loader.pending = 0xffff;
    loader.source = 0;
    if (pool_token && bank != 0xffff) select_bank_code(bank);
}

u16 codec_bank_claim(u16 token, u32 parameters[4])
{
    if (!pool_token || token != pool_token || loader.failed || loader.pending != 0xffff)
        return EXT_ERR_ARGUMENT;
    parameters[0] = loader.arena_begin;
    parameters[1] = loader.arena_end;
    parameters[2] = directory;
    parameters[3] = bank_count;
    loader.active = 0xffff;
    return EXT_OK;
}
