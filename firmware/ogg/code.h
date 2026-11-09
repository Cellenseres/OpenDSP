// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

typedef struct {
    u32 arena_begin, arena_end;
    u32 window_begin, window_end;
    u32 source, bytes, loads, loaded_bytes, copied;
    u16 active, pending, failed;
} ogg_code_loader;

int ogg_code_open(ogg_code_loader *loader, u32 arena_begin, u32 arena_end,
                  u32 window_begin, u32 window_end);
int ogg_code_load(ogg_code_loader *loader, u16 bank, u32 source, u32 bytes);
int ogg_code_begin(ogg_code_loader *loader, u16 bank, u32 source, u32 bytes);
int ogg_code_step(ogg_code_loader *loader);
void ogg_program_write(u16 destination, const u16 *source, u16 words, u16 page);
