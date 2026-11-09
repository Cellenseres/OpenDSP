// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>

typedef struct {
    u32 offset;
    u16 token;
    bool connected;
} codec_bank_pool;

u32 codec_banks_size(u32 capabilities);
Result codec_banks_register(codec_bank_pool *pool, void *memory, u32 capabilities);
Result codec_banks_upload(void);
Result codec_banks_upload_profile(u16 profile);
Result codec_banks_connect(codec_bank_pool *pool, void *memory, u32 capabilities, u16 codec);
Result codec_banks_disconnect(codec_bank_pool *pool);
