// SPDX-License-Identifier: BSD-2-Clause
#include <string.h>
#include "codec_banks_backend.h"
#include "codec_banks_protocol.h"
#include "codec_banks.h"
#include "extension.h"
#include "internal.h"
#include "opendsp.h"

#define DIRECTORY_BYTES ((CODEC_BANK_COUNT * CODEC_BANK_DESCRIPTOR_BYTES + 127u) & ~127u)
extern const u8 opendsp_codec_banks[CODEC_PAYLOAD_BYTES];

static void store_u32(u16 *words, u32 value)
{
    words[0] = value >> 16;
    words[1] = value;
}

u32 codec_banks_size(u32 capabilities)
{
    return capabilities & CODEC_CAP_BANKS ? DIRECTORY_BYTES + CODEC_PAYLOAD_BYTES : 0;
}

Result codec_banks_register(codec_bank_pool *pool, void *memory, u32 capabilities)
{
    u16 args[CODEC_BANK_CONFIG_WORDS];
    u32 physical, total = codec_banks_size(capabilities);
    u8 *directory;
    Result result;
    if (!total) return 0;
    if (!pool || !memory || !pool->offset || (pool->offset & 127) || !pool->token)
        return OPENDSP_ERR_ARGUMENT;
    directory = (u8 *)memory + pool->offset;
    physical = osConvertVirtToPhys(directory);
    if ((physical & 127) || physical < EXT_FCRAM_START || total > EXT_FCRAM_END - physical || physical >= EXT_FCRAM_END)
        return OPENDSP_ERR_ADDRESS;
    if (!pool->connected) {
        u16 *records = (u16 *)directory;
        memset(directory, 0, DIRECTORY_BYTES);
        memcpy(directory + DIRECTORY_BYTES, opendsp_codec_banks, CODEC_PAYLOAD_BYTES);
        for (u16 index = 0; index < CODEC_BANK_COUNT; ++index) {
            u32 offset = codec_banks[index].offset, bytes = codec_banks[index].bytes;
            if ((offset & 127) || offset > CODEC_PAYLOAD_BYTES || bytes > CODEC_PAYLOAD_BYTES - offset ||
                !bytes || (bytes & 1) || bytes > (0x20000u - CODEC_PROGRAM_WINDOW) * 2u)
                return OPENDSP_ERR_ARGUMENT;
            store_u32(records + index * 4, physical + DIRECTORY_BYTES + offset);
            store_u32(records + index * 4 + 2, bytes);
        }
        __dsb();
        DSP_FlushDataCache(directory, total);
        __dsb();
    }
    args[0] = CODEC_BANK_ABI;
    args[1] = pool->token;
    store_u32(args + 2, physical);
    store_u32(args + 4, physical + total);
    store_u32(args + 6, physical);
    args[8] = CODEC_BANK_COUNT;
    store_u32(args + 9, CODEC_PROGRAM_WINDOW);
    store_u32(args + 11, CODEC_IMAGE_TAG0);
    store_u32(args + 13, CODEC_IMAGE_TAG1);
    pool->connected = true;
    result = opendsp_request(CODEC_BANK_CONFIGURE, args, CODEC_BANK_CONFIG_WORDS, NULL, 0);
    opendsp_codec_error("Code pool configure", result);
    if (R_FAILED(result)) {
        if (result == OPENDSP_ERR_ARGUMENT || result == OPENDSP_ERR_ADDRESS || result == OPENDSP_ERR_UNSUPPORTED)
            pool->connected = false;
        return result;
    }
    return 0;
}

Result codec_banks_connect(codec_bank_pool *pool, void *memory, u32 capabilities, u16 codec)
{
    u16 args[2], status[5];
    if (codec > 2) return OPENDSP_ERR_ARGUMENT;
    Result result = codec_banks_register(pool, memory, capabilities);
    if (R_FAILED(result) || !codec_banks_size(capabilities)) return result;
    args[0] = pool->token;
    args[1] = codec ? CODEC_MP3_BANK : CODEC_AAC_BANK;
    result = opendsp_request(CODEC_BANK_SELECT, args, 2, NULL, 0);
    opendsp_codec_error("Code bank select", result);
    if (R_FAILED(result)) return result;
    u64 deadline = svcGetSystemTick() + SYSCLOCK_ARM11 / 4;
    for (;;) {
        result = opendsp_request(CODEC_BANK_STATUS, args, 1, status, 5);
        opendsp_codec_error("Code bank status", result);
        if (R_FAILED(result)) return result;
        if (status[2]) {
            opendsp_codec_error("Code bank DMA", OPENDSP_ERR_ADDRESS);
            return OPENDSP_ERR_ADDRESS;
        }
        if (status[0] == args[1] && status[1] == 0xffff) return 0;
        if (status[1] != args[1]) return OPENDSP_ERR_ARGUMENT;
        if (svcGetSystemTick() >= deadline) {
            opendsp_codec_error("Code bank load timeout", OPENDSP_ERR_TIMEOUT);
            return OPENDSP_ERR_TIMEOUT;
        }
        svcSleepThread(500000);
    }
}

Result codec_banks_disconnect(codec_bank_pool *pool)
{
    u16 args[1] = {pool->token};
    if (!pool->connected) return 0;
    Result result = opendsp_request(CODEC_BANK_CLOSE, args, 1, NULL, 0);
    opendsp_codec_error("Code pool close", result);
    if (R_SUCCEEDED(result)) pool->connected = false;
    return result;
}

Result codec_banks_upload(void)
{
    return codec_banks_upload_profile(1);
}

Result codec_banks_upload_profile(u16 profile)
{
    u32 addresses[CODEC_DATA_COUNT];
    if (profile < 1 || profile > 2) return OPENDSP_ERR_ARGUMENT;
#ifndef CODEC_DATA_PROFILES
    if (profile != 1) return OPENDSP_ERR_UNSUPPORTED;
#endif
    for (u16 index = 0; index < CODEC_DATA_COUNT; ++index) {
#ifdef CODEC_DATA_PROFILES
        if (codec_data_profiles[index] > 2) return OPENDSP_ERR_ARGUMENT;
        if (codec_data_profiles[index] && codec_data_profiles[index] != profile) continue;
#endif
        const codec_data_source *block = codec_data + index;
        u32 end = block->word_address + block->bytes / 2u;
        if (block->word_address < 0x10000u || block->word_address >= 0x20000u ||
            (block->bytes & 1) || block->bytes > (0x20000u - block->word_address) * 2u ||
            block->offset > CODEC_PAYLOAD_BYTES || block->bytes > CODEC_PAYLOAD_BYTES - block->offset)
            return OPENDSP_ERR_ARGUMENT;
        if (!((block->word_address >= 0x10040u && end <= 0x17000u) ||
              (block->word_address >= 0x18000u && end <= 0x18800u) ||
              (block->word_address >= 0x1b080u && end <= 0x20000u))) return OPENDSP_ERR_ADDRESS;
        Result result = DSP_ConvertProcessAddressFromDspDram(block->word_address, addresses + index);
        opendsp_codec_error("DSP private data map", result);
        if (R_FAILED(result)) return result;
    }
    for (u16 phase = 0; phase <= 1; ++phase)
        for (u16 index = 0; index < CODEC_DATA_COUNT; ++index) {
#ifdef CODEC_DATA_PROFILES
            u16 selected = codec_data_profiles[index];
            if (selected != (phase ? profile : 0)) continue;
#else
            if (phase) continue;
#endif
            memcpy((void *)(uintptr_t)addresses[index], opendsp_codec_banks + codec_data[index].offset,
                   codec_data[index].bytes);
        }
    __dsb();
    return 0;
}
