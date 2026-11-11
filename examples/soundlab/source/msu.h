// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>
#include "wav.h"

#define MSU_DIR WAV_DIR "/msu"

// Drives the MSU-1 registers like an emulator would.
bool msu_open(char *error, size_t length);
void msu_close(void);
bool msu_ready(void);
const char *msu_pack(void);
int msu_track(void);
bool msu_track_present(int track);
void msu_step_track(int delta, bool any);
void msu_load(void);
void msu_play_pause(void);
void msu_toggle_repeat(void);
bool msu_repeat(void);
void msu_stop_bookmark(void);
int msu_volume(void);
void msu_set_volume(int volume);
void msu_fade(void);
void msu_reset(void);
void msu_apply(void);
void msu_update(void); // once per frame
u8 msu_status(void);
void msu_id(char out[7]);
bool msu_playing(void);
opendspAudioStream *msu_stream(void); // NULL without a track
const char *msu_codec(void);
bool msu_times(float *position, float *length, float *loop); // seconds
