// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

typedef struct {
    u32 address, bytes, arena_begin, arena_end;
    u32 cache_address;
    u16 cache_bytes, failed;
    u16 cache[64];
} ogg_input;

int ogg_input_open(ogg_input *input, u32 address, u32 bytes, u32 arena_begin, u32 arena_end);
int ogg_input_byte(void *context, long offset);
