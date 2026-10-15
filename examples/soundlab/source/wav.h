// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>

#define WAV_DIR "sdmc:/3ds/opendsp-lab"

int  wav_scan(void);                     // returns the file count
int  wav_count(void);
const char *wav_name(int i);
bool wav_play(int i, char *err, size_t errlen);
void wav_stop(void);
bool wav_playing(void);
void wav_set_loop(bool loop);
bool wav_loop(void);
void wav_apply(void);                    // apply speed and interpolation
const char *wav_info(void);              // e.g. "44100 Hz stereo PCM16"
float wav_progress(void);                // 0..1
void wav_exit(void);
