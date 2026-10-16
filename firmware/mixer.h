// SPDX-License-Identifier: BSD-2-Clause
// Mixing buses, aux, delay, output stage.
#pragma once
#include "voice.h"

// main, mix 1 and 2 front/rear, main rear, aux front after surround
#define NUM_BUSES 7
#define BUS_MAIN_REAR 5
#define BUS_POST 6

typedef struct { s32 *acc; u16 pad0; const s16 *src; u16 pad1; u16 gain_l, gain_r, count; } mix_job_t;
typedef struct { s16 *buf; u16 pad0; s16 *state; u16 pad1; const s16 *coefs; u16 pad2; } filter_job_t;

void mix_add(const mix_job_t *job);
typedef struct { s32 *acc; u16 pad0; const s16 *src; u16 pad1; const s16 *gains; } mix_ramp_job_t;
void mix_ramp(const mix_ramp_job_t *job);
void filter_simple(const filter_job_t *job);
void filter_biquad(const filter_job_t *job);

void mixer_init(void);
u16 mixer_parse(vu16 *config);
u16 mixer_route(u16 mix);
u16 mixer_rear_mode(void);
s16 mixer_volume(u16 mix);
void mixer_begin(void);
void mixer_add_source(const s16 *src, s16 gain[NUM_BUSES][2], s16 previous[NUM_BUSES][2], u16 fresh);
void mixer_end(u16 read_bank, u16 write_bank, vu16 *out);
// coefficients Q7; address of frames * 1280 bytes
typedef struct { u16 enable, frames, channels; s16 g, a, b; u16 zeroed, pad; u32 address; } delay_params_t;
u16 mixer_set_delay(u16 bus, const delay_params_t *params);
