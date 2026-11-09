// SPDX-License-Identifier: BSD-2-Clause
#include "decoder.h"
#include "task.h"
#ifdef memset
#undef memset
#endif
#include <stddef.h>
#include <string.h>

void clear_words(void *destination, u16 words);
void ogg_opus_poll(void);

void *vorbis_memset(void *destination, int value, size_t bytes)
{
    u16 *target = destination;
    if (value || (bytes & 1)) return memset(destination, value, bytes);
    size_t words = bytes / 2;
    while (words) {
        u16 count = words > 128 ? 128 : (u16)words;
        clear_words(target, count);
        target += count;
        words -= count;
        ogg_opus_poll();
    }
    return destination;
}

void abort(void)
{
    for (;;) {
        ((vu16 *)OGG_DECODE_WORD)[OGG_TASK_ERROR] = 2;
        ogg_task_yield();
    }
}

int abs(int value)
{
    return value < 0 ? -value : value;
}

long labs(long value)
{
    return value < 0 ? -value : value;
}
