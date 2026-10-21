/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once
#include <opendsp/mp3.h>

typedef struct mp3_arm11 mp3_arm11;
mp3_arm11 *mp3_arm11_open(void);
void mp3_arm11_reset(mp3_arm11 *d);
void mp3_arm11_close(mp3_arm11 *d);
size_t mp3_arm11_bytes(void);
Result mp3_arm11_decode(mp3_arm11 *d, const void *input, size_t bytes, size_t *used, s16 *pcm,
                        size_t capacity, size_t *frames, opendspMp3Info *info);
