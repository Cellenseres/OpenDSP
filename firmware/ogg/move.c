// SPDX-License-Identifier: BSD-2-Clause
#include "hw.h"
#include "decoder.h"

void ogg_copy_forward(const u16 *source, u16 *destination, u16 words);
void ogg_copy_backward(const u16 *source, u16 *destination, u16 words);
void ogg_opus_poll(void);

void ogg_move_words(u16 *destination, const u16 *source, u16 words)
{
    if (destination == source) return;
    if ((u16)destination < (u16)source) {
        while (words) {
            u16 count = words > 128 ? 128 : words;
            ogg_copy_forward(source, destination, count);
            source += count;
            destination += count;
            words -= count;
            ogg_opus_poll();
        }
    } else {
        source += words;
        destination += words;
        while (words) {
            u16 count = words > 128 ? 128 : words;
            source -= count;
            destination -= count;
            ogg_copy_backward(source, destination, count);
            words -= count;
            ogg_opus_poll();
        }
    }
}
