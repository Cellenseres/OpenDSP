// SPDX-License-Identifier: BSD-2-Clause
#include "memory.h"
#include "schedule.h"

void clear_words(void *destination, u16 words);

void ogg_zero_words(void *destination, u32 words)
{
    u16 *target = destination;
    while (words) {
        u16 count = words > 128 ? 128 : (u16)words;
        clear_words(target, count);
        target += count;
        words -= count;
        tremor_decode_yield();
    }
}
