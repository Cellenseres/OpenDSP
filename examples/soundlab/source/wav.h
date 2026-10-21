// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>
#include <opendsp/audio.h>

#define WAV_DIR "sdmc:/3ds/opendsp-lab"

int wav_scan(void);
int wav_count(void);
const char *wav_name(int i);
bool wav_play(int i, char *err, size_t errlen);
void wav_stop(void);
bool wav_playing(void);
void wav_set_loop(bool loop);
bool wav_loop(void);
void wav_apply(void);
const char *wav_status(void);
const char *wav_info(void);
float wav_progress(void); // 0..1; -1 while duration is unknown
void wav_exit(void);
void wav_frame_notify(void);
void wav_set_backend(unsigned backend);
unsigned wav_backend(void);
unsigned wav_active_backend(void); // AUTO for PCM/idle
const char *wav_error(void);

bool wav_stats(opendspAudioStats *stats);
bool wav_stream_info(opendspAudioInfo *info);
bool wav_compare_backend(char *error, size_t length);
bool wav_select_backend(unsigned backend, char *error, size_t length);
