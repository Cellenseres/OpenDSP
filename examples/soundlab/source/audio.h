// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>

// channel map
#define CH_LEAD    0
#define CH_ARP     1
#define CH_BASS    2
#define CH_KICK    3
#define CH_SNARE   4
#define CH_HAT     5
#define CH_SFX0    6
#define CH_NSFX    6
#define CH_UI      12
#define CH_WAV     13
#define CH_KEY0    14
#define CH_NKEY    4
#define NUM_TRACKS 6

typedef struct {
    int master;      // 0..100 %
    int output;      // 0 mono, 1 stereo, 2 surround
    int clip;        // 0 normal, 1 soft
    int interp;      // 0 polyphase, 1 linear, 2 none
    int speed;       // 50..200 %
    int filter;      // see audio_filter_name()
    int cutoff;      // Hz
    int pan;         // -100..100
    int surround;    // 0..100 %
    int ui_sounds;
    int key_wave;
    int key_octave;
    int reverb;         // preset, 0 off
    int reverb_send;    // 0..100 %
    int reverb_return;  // 0..100 %
    int reverb_decay;   // tenths of a second
    int delay;          // 0 off, 1 on
    int delay_ms;
    int delay_feedback; // 0..90 %
} settings_t;
extern settings_t S;

bool audio_init(char *error, size_t error_len);
void audio_exit(void);
void audio_apply_settings(void);
void audio_apply_effects(void);
void audio_build_mix(int ch, float l, float r, float mix[12]);
void audio_set_mix(int ch, float l, float r);
const char *audio_reverb_name(int preset);
int audio_reverb_count(void);
float audio_reverb_default_decay(int preset);
bool audio_effects_available(void);
u16 audio_firmware_version(void);
const char *audio_filter_name(int f);
int audio_filter_count(void);
const char *audio_wave_name(int w);
int audio_wave_count(void);

void music_play(bool on);
bool music_is_playing(void);
int music_bpm(void);
void music_set_bpm(int bpm);
bool music_muted(int track);
void music_set_muted(int track, bool mute);
const char *music_track_name(int track);
const char *music_track_format(int track);
int music_bar(void);
int music_step(void);
const char *music_chord(void);

int sfx_count(void);
const char *sfx_name(int i);
const char *sfx_format(int i);
void sfx_play(int i);
void ui_blip(int kind);          // 0 move, 1 select, 2 back

void key_on(int slot, int midi);
void key_off(int slot);

void lab_chord(bool on);
bool lab_chord_on(void);
void lab_burst(void);
void lab_formats(void);
void stop_all(void);

u32 audio_frames(void);
u32 audio_dropped(void);
const s16 *audio_capture(u32 *position, u32 *length);
