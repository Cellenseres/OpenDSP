// SPDX-License-Identifier: BSD-2-Clause
// Sound Lab audio: sounds, song, sequencer.
#include <3ds.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opendsp.h>
#include <opendsp/effects.h>
#include "audio.h"

#define SAMPLE_RATE 32728.0f
#define TABLE_LEN   64
#define PI_F        3.14159265f

settings_t S = {80, 1, 1, 0, 100, 0, 1500, 0, 50, 1, 3, 4, 2, 35, 70, 26, 0, 300, 35};

// sounds

enum { W_SINE, W_TRIANGLE, W_SAW, W_SQUARE, W_PULSE, W_COUNT };
static const char *wave_names[W_COUNT] = {"Sine", "Triangle", "Saw", "Square", "Pulse 25%"};
static s16 *tables[W_COUNT];

typedef struct {
    const char *name, *format_name;
    void *data;
    u32 length;
    u16 format;
    ndspAdpcmData adpcm;
    float volume;
} sample_t;

enum { D_KICK, D_SNARE, D_HAT, D_COUNT };
static sample_t drums[D_COUNT];

enum { FX_LASER, FX_COIN, FX_JUMP, FX_EXPLOSION, FX_POWERUP, FX_ALARM, FX_CHIME, FX_CHIME8, FX_CHIME_ADPCM, FX_COUNT };
static sample_t sfx[FX_COUNT];
static sample_t blips[3];

// DSP-ADPCM predictor pairs (Q11)
static const s16 adpcm_coefs[16] = {0, 0, 2048, 0, 4096, -2048, 3900, -1950, 3000, -1000, 1024, 0, 3584, -1792, 4000, -2016};

static float noise(void) { return (float)rand() / (float)RAND_MAX * 2.f - 1.f; }
static s16 clamp16(float v) { return v > 32767.f ? 32767 : v < -32768.f ? -32768 : (s16)v; }

static s16 *alloc16(u32 n)
{
    s16 *p = linearAlloc(n * 2);
    if (p)
        memset(p, 0, n * 2);
    return p;
}

static void make_tables(void)
{
    for (int w = 0; w < W_COUNT; w++) {
        tables[w] = alloc16(TABLE_LEN);
        for (int i = 0; i < TABLE_LEN; i++) {
            float p = (float)i / TABLE_LEN, v = 0;
            switch (w) {
            case W_SINE:     v = sinf(2 * PI_F * p); break;
            case W_TRIANGLE: v = p < 0.5f ? 4 * p - 1 : 3 - 4 * p; break;
            case W_SAW:      v = 2 * p - 1; break;
            case W_SQUARE:   v = p < 0.5f ? 0.8f : -0.8f; break;
            case W_PULSE:    v = p < 0.25f ? 0.8f : -0.8f; break;
            }
            tables[w][i] = clamp16(v * 12000.f);
        }
        DSP_FlushDataCache(tables[w], TABLE_LEN * 2);
    }
}

static void finish_pcm16(sample_t *s, const char *name, s16 *data, u32 length, float volume)
{
    DSP_FlushDataCache(data, length * 2);
    s->name = name;
    s->format_name = "PCM16";
    s->data = data;
    s->length = length;
    s->format = NDSP_FORMAT_MONO_PCM16;
    s->volume = volume;
}

static void to_pcm8(sample_t *dst, const sample_t *src, const char *name)
{
    s8 *d = linearAlloc(src->length);
    const s16 *in = src->data;
    for (u32 i = 0; i < src->length; i++)
        d[i] = (s8)(in[i] >> 8);
    DSP_FlushDataCache(d, src->length);
    *dst = *src;
    dst->name = name;
    dst->format_name = "PCM8";
    dst->data = d;
    dst->format = NDSP_FORMAT_MONO_PCM8;
}

static s16 adpcm_sample(int nibble, int scale, int c1, int c2, int y1, int y2)
{
    s32 x = ((nibble * (1 << scale)) << 11) + 1024 + c1 * y1 + c2 * y2;
    x >>= 11;
    return x > 32767 ? 32767 : x < -32768 ? -32768 : x;
}

// per frame: keep the best predictor
static void to_adpcm(sample_t *dst, const sample_t *src, const char *name)
{
    const s16 *in = src->data;
    u32 frames = (src->length + 13) / 14;
    u8 *out = linearAlloc(frames * 8);
    int y1 = 0, y2 = 0;
    for (u32 f = 0; f < frames; f++) {
        long best = -1;
        u8 best_frame[8] = {0};
        int best_y1 = 0, best_y2 = 0;
        for (int p = 0; p < 8; p++) {
            int c1 = adpcm_coefs[2 * p], c2 = adpcm_coefs[2 * p + 1];
            int a = y1, b = y2, peak = 0;
            for (int i = 0; i < 14; i++) {
                int want = f * 14 + i < src->length ? in[f * 14 + i] : 0;
                int r = want - ((c1 * a + c2 * b) >> 11);
                if (abs(r) > peak)
                    peak = abs(r);
                b = a;
                a = want;
            }
            int first_scale = 0;
            while (first_scale < 12 && (7 << first_scale) < peak)
                first_scale++;
            for (int scale = first_scale; scale <= first_scale + 1 && scale <= 12; scale++) {
                u8 frame[8] = {(u8)(p << 4 | scale)};
                long error = 0;
                a = y1;
                b = y2;
                for (int i = 0; i < 14; i++) {
                    int want = f * 14 + i < src->length ? in[f * 14 + i] : 0;
                    int predicted = (c1 * a + c2 * b + 1024) >> 11;
                    int guess = (int)lroundf((float)(want - predicted) / (float)(1 << scale));
                    if (guess > 7)
                        guess = 7;
                    if (guess < -8)
                        guess = -8;
                    int nibble = guess;
                    long nibble_error = -1;
                    s16 value = 0;
                    for (int t = guess - 1; t <= guess + 1; t++) {
                        if (t < -8 || t > 7)
                            continue;
                        s16 v = adpcm_sample(t, scale, c1, c2, a, b);
                        long e = (long)(v - want) * (v - want);
                        if (nibble_error < 0 || e < nibble_error) {
                            nibble_error = e;
                            nibble = t;
                            value = v;
                        }
                    }
                    error += nibble_error;
                    frame[1 + i / 2] |= (u8)((nibble & 15) << ((i & 1) ? 0 : 4));
                    b = a;
                    a = value;
                }
                if (best < 0 || error < best) {
                    best = error;
                    memcpy(best_frame, frame, 8);
                    best_y1 = a;
                    best_y2 = b;
                }
            }
        }
        memcpy(out + f * 8, best_frame, 8);
        y1 = best_y1;
        y2 = best_y2;
    }
    DSP_FlushDataCache(out, frames * 8);
    *dst = *src;
    dst->name = name;
    dst->format_name = "DSP-ADPCM";
    dst->data = out;
    dst->format = NDSP_FORMAT_MONO_ADPCM;
    dst->adpcm.index = out[0];
    dst->adpcm.history0 = 0;
    dst->adpcm.history1 = 0;
}

static void make_drums(void)
{
    u32 n = (u32)(0.35f * SAMPLE_RATE);
    s16 *d = alloc16(n);
    float phase = 0;
    for (u32 i = 0; i < n; i++) {               // kick
        float t = i / SAMPLE_RATE, f = 45.f + 115.f * expf(-t * 28.f);
        phase += 2 * PI_F * f / SAMPLE_RATE;
        d[i] = clamp16(sinf(phase) * 26000.f * expf(-t * 9.f) + (i < 40 ? noise() * 6000.f : 0));
    }
    finish_pcm16(&drums[D_KICK], "Kick", d, n, 1.0f);

    sample_t tmp;
    n = (u32)(0.25f * SAMPLE_RATE);
    d = alloc16(n);
    phase = 0;
    for (u32 i = 0; i < n; i++) {               // snare
        float t = i / SAMPLE_RATE;
        phase += 2 * PI_F * 190.f / SAMPLE_RATE;
        d[i] = clamp16(noise() * 16000.f * expf(-t * 16.f) + sinf(phase) * 9000.f * expf(-t * 22.f));
    }
    finish_pcm16(&tmp, "Snare", d, n, 0.8f);
    to_pcm8(&drums[D_SNARE], &tmp, "Snare");
    linearFree(d);

    n = (u32)(0.07f * SAMPLE_RATE);
    d = alloc16(n);
    float last = 0;
    for (u32 i = 0; i < n; i++) {               // hi-hat
        float t = i / SAMPLE_RATE, x = noise();
        d[i] = clamp16((x - last) * 9000.f * expf(-t * 60.f));
        last = x;
    }
    finish_pcm16(&tmp, "Hi-hat", d, n, 0.5f);
    to_adpcm(&drums[D_HAT], &tmp, "Hi-hat");
    linearFree(d);
}

static void make_sfx(void)
{
    u32 n;
    s16 *d;
    float phase = 0;

    n = (u32)(0.30f * SAMPLE_RATE);
    d = alloc16(n);
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE, f = 200.f + 1600.f * expf(-t * 12.f);
        phase += f / SAMPLE_RATE;
        phase -= floorf(phase);
        d[i] = clamp16((phase < 0.5f ? 1 : -1) * 9000.f * (1 - t / 0.30f));
    }
    finish_pcm16(&sfx[FX_LASER], "Laser", d, n, 0.7f);

    n = (u32)(0.40f * SAMPLE_RATE);
    d = alloc16(n);
    phase = 0;
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE, f = t < 0.07f ? 987.8f : 1318.5f;
        phase += f / SAMPLE_RATE;
        phase -= floorf(phase);
        d[i] = clamp16((phase < 0.5f ? 1 : -1) * 8000.f * (t < 0.07f ? 1 : expf(-(t - 0.07f) * 7.f)));
    }
    finish_pcm16(&sfx[FX_COIN], "Coin", d, n, 0.7f);

    n = (u32)(0.28f * SAMPLE_RATE);
    d = alloc16(n);
    phase = 0;
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE, f = 260.f + 900.f * t / 0.28f;
        phase += f / SAMPLE_RATE;
        phase -= floorf(phase);
        float tri = phase < 0.5f ? 4 * phase - 1 : 3 - 4 * phase;
        d[i] = clamp16(tri * 14000.f * (1 - t / 0.28f));
    }
    finish_pcm16(&sfx[FX_JUMP], "Jump", d, n, 0.8f);

    sample_t tmp;
    n = (u32)(1.1f * SAMPLE_RATE);
    d = alloc16(n);
    float lp = 0, lp2 = 0;
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE, k = 0.02f + 0.25f * expf(-t * 4.f);
        lp += k * (noise() - lp);
        lp2 += k * (lp - lp2);
        d[i] = clamp16(lp2 * 60000.f * expf(-t * 2.8f));
    }
    finish_pcm16(&tmp, "Explosion", d, n, 1.0f);
    to_pcm8(&sfx[FX_EXPLOSION], &tmp, "Explosion");
    linearFree(d);

    n = (u32)(0.62f * SAMPLE_RATE);
    d = alloc16(n);
    phase = 0;
    static const float arpeggio[8] = {523.3f, 659.3f, 784.0f, 1046.5f, 659.3f, 784.0f, 1046.5f, 1318.5f};
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE;
        int k = (int)(t / 0.0775f);
        if (k > 7)
            k = 7;
        phase += arpeggio[k] / SAMPLE_RATE;
        phase -= floorf(phase);
        d[i] = clamp16((phase < 0.25f ? 1 : -0.33f) * 9000.f * (1 - t / 0.62f * 0.6f));
    }
    finish_pcm16(&sfx[FX_POWERUP], "Power-up", d, n, 0.6f);

    n = (u32)(0.9f * SAMPLE_RATE);
    d = alloc16(n);
    phase = 0;
    for (u32 i = 0; i < n; i++) {
        float t = i / SAMPLE_RATE, f = fmodf(t, 0.3f) < 0.15f ? 880.f : 660.f;
        phase += f / SAMPLE_RATE;
        phase -= floorf(phase);
        d[i] = clamp16(sinf(2 * PI_F * phase) * 12000.f * (t < 0.8f ? 1 : (0.9f - t) * 10));
    }
    finish_pcm16(&sfx[FX_ALARM], "Alarm", d, n, 0.6f);

    n = (u32)(1.3f * SAMPLE_RATE);
    d = alloc16(n);
    for (u32 i = 0; i < n; i++) {               // bell partials
        float t = i / SAMPLE_RATE;
        float v = sinf(2 * PI_F * 880.f * t) * expf(-t * 2.5f) + 0.5f * sinf(2 * PI_F * 1763.f * t) * expf(-t * 4.f)
                + 0.3f * sinf(2 * PI_F * 2642.f * t) * expf(-t * 6.f) + 0.2f * sinf(2 * PI_F * 4410.f * t) * expf(-t * 9.f);
        d[i] = clamp16(v * 13000.f * (i < 60 ? i / 60.f : 1));
    }
    finish_pcm16(&sfx[FX_CHIME], "Chime", d, n, 0.8f);
    to_pcm8(&sfx[FX_CHIME8], &sfx[FX_CHIME], "Chime");
    to_adpcm(&sfx[FX_CHIME_ADPCM], &sfx[FX_CHIME], "Chime");

    for (int b = 0; b < 3; b++) {               // menu move, select, back
        static const float pitch[3] = {1800.f, 2400.f, 1200.f};
        n = (u32)(0.035f * SAMPLE_RATE);
        d = alloc16(n);
        for (u32 i = 0; i < n; i++)
            d[i] = clamp16(sinf(2 * PI_F * pitch[b] * i / SAMPLE_RATE) * 7000.f * (1 - (float)i / n));
        finish_pcm16(&blips[b], "Blip", d, n, 0.35f);
    }
}

// channels

typedef struct {
    bool running;                // looping table queued
    int wave;
    int stage;                   // off, attack, decay, sustain, release
    float level, peak, sustain, attack, decay, release;
    float volume, pan;
    int gate;                    // frames until release, -1 = held
    bool music;                  // follows the music pan
    float last_l, last_r;
} channel_t;

static channel_t channels[24];
static ndspWaveBuf wavebufs[24];
static LightLock lock;

static float note_hz(int midi) { return 440.f * powf(2.f, (midi - 69) / 12.f); }

// main, plus sends: aux 0 reverb, aux 1 delay
void audio_set_mix(int ch, float l, float r)
{
    float mix[12] = {0};
    float reverb = S.reverb ? S.reverb_send / 100.f : 0, delay = S.delay ? 0.5f : 0;
    mix[0] = l;
    mix[1] = r;
    if (ch != CH_UI) {
        mix[4] = l * reverb;
        mix[5] = r * reverb;
        mix[8] = l * delay;
        mix[9] = r * delay;
    }
    ndspChnSetMix(ch, mix);
}

static void pan_gains(float pan, float *l, float *r)
{
    if (pan > 1)
        pan = 1;
    if (pan < -1)
        pan = -1;
    *l = pan > 0 ? 1 - pan : 1;
    *r = pan < 0 ? 1 + pan : 1;
}

static void prepare_channel(int ch, u16 format)
{
    ndspChnWaveBufClear(ch);
    ndspChnSetInterp(ch, S.interp);
    ndspChnSetFormat(ch, format);
}

// envelope times in DSP frames (4.9 ms)
static void tone_on(int ch, int wave, float freq, float volume, float pan, int attack, int decay,
                    float sustain, int release, int gate, bool music)
{
    channel_t *c = &channels[ch];
    if (!c->running || c->wave != wave) {
        prepare_channel(ch, NDSP_FORMAT_MONO_PCM16);
        audio_set_mix(ch, 0, 0);        // reset leaves full volume until the envelope runs
        memset(&wavebufs[ch], 0, sizeof wavebufs[ch]);
        wavebufs[ch].data_pcm16 = tables[wave];
        wavebufs[ch].nsamples = TABLE_LEN;
        wavebufs[ch].looping = true;
        ndspChnWaveBufAdd(ch, &wavebufs[ch]);
        c->running = true;
        c->wave = wave;
        c->level = 0;
        c->last_l = c->last_r = -1;
    }
    ndspChnSetRate(ch, freq * TABLE_LEN);
    c->peak = 1.0f;
    c->sustain = sustain;
    c->attack = 1.0f / (attack > 0 ? attack : 1);
    c->decay = (1.0f - sustain) / (decay > 0 ? decay : 1);
    c->release = 1.0f / (release > 0 ? release : 1);
    c->volume = volume;
    c->pan = pan;
    c->gate = gate;
    c->music = music;
    c->stage = 1;
}

static void tone_off(int ch)
{
    if (channels[ch].stage && channels[ch].stage != 4)
        channels[ch].stage = 4;
}

static void update_channel(int ch)
{
    channel_t *c = &channels[ch];
    if (!c->stage)
        return;
    switch (c->stage) {
    case 1:
        c->level += c->attack;
        if (c->level >= c->peak) {
            c->level = c->peak;
            c->stage = 2;
        }
        break;
    case 2:
        c->level -= c->decay;
        if (c->level <= c->sustain) {
            c->level = c->sustain;
            c->stage = 3;
        }
        break;
    case 4:
        c->level -= c->release;
        if (c->level <= 0)
            c->level = 0;
        break;
    }
    if (c->gate > 0 && --c->gate == 0 && c->stage != 4)
        c->stage = 4;
    float pl, pr;
    pan_gains(c->pan + (c->music ? S.pan / 100.f : 0), &pl, &pr);
    float l = c->level * c->volume * pl, r = c->level * c->volume * pr;
    if (fabsf(l - c->last_l) > 0.0005f || fabsf(r - c->last_r) > 0.0005f) {
        audio_set_mix(ch, l, r);
        c->last_l = l;
        c->last_r = r;
    }
    if (c->stage == 4 && c->level <= 0) {
        c->stage = 0;
        ndspChnWaveBufClear(ch);
        c->running = false;
    }
}

static void play_sample(int ch, const sample_t *s, float rate, float volume, float pan)
{
    channel_t *c = &channels[ch];
    c->stage = 0;
    c->running = false;
    prepare_channel(ch, s->format);
    if (s->format == NDSP_FORMAT_MONO_ADPCM)
        ndspChnSetAdpcmCoefs(ch, (u16 *)adpcm_coefs);
    ndspChnSetRate(ch, SAMPLE_RATE * rate);
    float pl, pr;
    pan_gains(pan, &pl, &pr);
    audio_set_mix(ch, volume * s->volume * pl, volume * s->volume * pr);
    memset(&wavebufs[ch], 0, sizeof wavebufs[ch]);
    wavebufs[ch].data_vaddr = s->data;
    wavebufs[ch].nsamples = s->length;
    wavebufs[ch].looping = false;
    wavebufs[ch].adpcm_data = s->format == NDSP_FORMAT_MONO_ADPCM ? (ndspAdpcmData *)&s->adpcm : NULL;
    ndspChnWaveBufAdd(ch, &wavebufs[ch]);
}

// "Teak Street", 8 bars in A minor
// lead: MIDI note, 1 hold, 0 rest

static const u8 lead[8][16] = {
    {76, 1, 1, 1, 72, 1, 74, 1, 76, 1, 1, 1, 79, 1, 76, 1},
    {77, 1, 1, 1, 76, 1, 74, 1, 72, 1, 1, 1, 0, 0, 69, 1},
    {72, 1, 74, 1, 76, 1, 79, 1, 81, 1, 1, 1, 79, 1, 76, 1},
    {74, 1, 1, 1, 1, 1, 0, 0, 71, 1, 74, 1, 79, 1, 1, 1},
    {76, 1, 1, 1, 72, 1, 74, 1, 76, 1, 1, 1, 81, 1, 79, 1},
    {77, 1, 1, 1, 81, 1, 79, 1, 77, 1, 76, 1, 74, 1, 72, 1},
    {74, 1, 76, 1, 79, 1, 1, 1, 83, 1, 1, 1, 81, 1, 79, 1},
    {80, 1, 1, 1, 1, 1, 1, 1, 76, 1, 1, 1, 0, 0, 0, 0},
};
static const u8 roots[8] = {45, 41, 48, 43, 45, 41, 43, 40};
static const char *chord_names[8] = {"Am", "F", "C", "G", "Am", "F", "G", "E"};
static const u8 chord_tones[8][4] = {
    {57, 60, 64, 69}, {53, 57, 60, 65}, {55, 60, 64, 67}, {55, 59, 62, 67},
    {57, 60, 64, 69}, {53, 57, 60, 65}, {55, 59, 62, 67}, {56, 59, 64, 68},
};
static const s8 bass_pattern[16] = {0, -1, 12, -1, -1, 0, -1, 12, 0, -1, 12, -1, 7, -1, 12, -1};
static const u8 kick_pattern[16] = {1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};
static const u8 snare_pattern[16] = {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};
static const char *track_names[NUM_TRACKS] = {"Lead", "Arpeggio", "Bass", "Kick", "Snare", "Hi-hat"};
static const char *track_formats[NUM_TRACKS] = {"Pulse 25%", "Square", "Triangle", "PCM16", "PCM8", "DSP-ADPCM"};

static volatile bool music_on;
static volatile int tempo = 124;
static volatile bool muted[NUM_TRACKS];
static volatile int bar, step;
static float step_clock;

static void play_step(int b, int s)
{
    float pitch = S.speed / 100.f;
    int note = lead[b][s];
    if (!muted[0]) {
        if (note >= 2)
            tone_on(CH_LEAD, W_PULSE, note_hz(note) * pitch, 0.30f, 0.15f, 2, 30, 0.6f, 12, -1, true);
        else if (note == 0)
            tone_off(CH_LEAD);
    }
    if (!muted[1])
        tone_on(CH_ARP, W_SQUARE, note_hz(chord_tones[b][s & 3] + 12) * pitch, 0.10f, -0.35f, 1, 8, 0.3f, 6, 4, true);
    if (!muted[2] && bass_pattern[s] >= 0)
        tone_on(CH_BASS, W_TRIANGLE, note_hz(roots[b] + bass_pattern[s]) * pitch, 0.55f, 0, 1, 10, 0.7f, 8, 10, true);
    float pan = S.pan / 100.f;
    bool fill = b == 7 && s >= 13;
    if (!muted[3] && kick_pattern[s])
        play_sample(CH_KICK, &drums[D_KICK], pitch, 0.9f, pan);
    if (!muted[4] && (snare_pattern[s] || fill))
        play_sample(CH_SNARE, &drums[D_SNARE], pitch, fill ? 0.6f : 0.85f, pan + 0.1f);
    if (!muted[5] && !(s & 1))
        play_sample(CH_HAT, &drums[D_HAT], pitch, (s & 3) ? 0.45f : 0.7f, pan - 0.25f);
}

// DSP lab
static volatile bool pad_on;
static volatile int burst_left, burst_wait;
static volatile int formats_frame = -1;

// sequencer thread, woken once per DSP frame
static LightEvent frame_event;
static volatile u32 frames_seen;
static volatile bool sequencer_running;
static Thread sequencer;
static ndspWaveBuf capture;
static s16 *capture_buf;
#define CAPTURE_FRAMES (160 * 8)

static void on_frame(void *arg)
{
    (void)arg;
    frames_seen++;
    LightEvent_Signal(&frame_event);
}

static void process_frame(void)
{
    LightLock_Lock(&lock);
    if (music_on) {
        float samples_per_step = SAMPLE_RATE * 60.f / (tempo * 4.f);
        if (step_clock < 0)
            step_clock = samples_per_step;
        else
            step_clock += 160.f;
        while (step_clock >= samples_per_step) {
            step_clock -= samples_per_step;
            play_step(bar, step);
            if (++step == 16) {
                step = 0;
                bar = (bar + 1) & 7;
            }
        }
    }
    if (burst_left > 0 && --burst_wait <= 0) {
        int k = burst_left--;
        play_sample(CH_SFX0 + (k % CH_NSFX), k & 1 ? &sfx[FX_COIN] : &blips[1], 1.0f + (k % 5) * 0.12f,
                    0.6f, (k % 3 - 1) * 0.6f);
        burst_wait = 3;
    }
    if (formats_frame >= 0) {
        if (formats_frame == 0)
            play_sample(CH_SFX0, &sfx[FX_CHIME], 1.0f, 1.0f, 0);
        if (formats_frame == 290)
            play_sample(CH_SFX0 + 1, &sfx[FX_CHIME8], 1.0f, 1.0f, 0);
        if (formats_frame == 580) {
            play_sample(CH_SFX0 + 2, &sfx[FX_CHIME_ADPCM], 1.0f, 1.0f, 0);
            formats_frame = -2;
        }
        formats_frame++;
    }
    for (int ch = 0; ch < 24; ch++)
        update_channel(ch);
    LightLock_Unlock(&lock);
}

static void sequencer_main(void *arg)
{
    (void)arg;
    u32 done = frames_seen;
    while (sequencer_running) {
        LightEvent_WaitTimeout(&frame_event, 50000000LL);
        u32 now = frames_seen;
        int n = (int)(now - done);
        if (n > 4)                      // fell behind: skip ahead
            n = 4;
        for (int k = 0; k < n; k++)
            process_frame();
        done = now;
    }
}

// public

const char *audio_filter_name(int f)
{
    static const char *names[] = {"Off", "Low-pass (1-pole)", "High-pass (1-pole)", "Low-pass (biquad)",
                                  "High-pass (biquad)", "Band-pass (biquad)", "Notch (biquad)"};
    return f >= 0 && f < (int)(sizeof names / sizeof names[0]) ? names[f] : "?";
}

int audio_filter_count(void) { return 7; }
const char *audio_wave_name(int w) { return w >= 0 && w < W_COUNT ? wave_names[w] : "?"; }
int audio_wave_count(void) { return W_COUNT; }

static void apply_filter(int ch)
{
    float f = (float)S.cutoff;
    ndspChnIirMonoSetEnable(ch, false);
    ndspChnIirBiquadSetEnable(ch, false);
    switch (S.filter) {
    case 1: ndspChnIirMonoSetParamsLowPassFilter(ch, f); break;
    case 2: ndspChnIirMonoSetParamsHighPassFilter(ch, f); break;
    case 3: ndspChnIirBiquadSetParamsLowPassFilter(ch, f, 0.9f); break;
    case 4: ndspChnIirBiquadSetParamsHighPassFilter(ch, f, 0.9f); break;
    case 5: ndspChnIirBiquadSetParamsBandPassFilter(ch, f, 1.2f); break;
    case 6: ndspChnIirBiquadSetParamsNotchFilter(ch, f, 0.8f); break;
    }
}

void audio_apply_settings(void)
{
    ndspSetMasterVol(S.master / 100.f);
    ndspSetOutputMode((ndspOutputMode)S.output);
    ndspSetClippingMode((ndspClippingMode)S.clip);
    ndspSurroundSetDepth((u16)(S.surround * 0x7FFF / 100));
    for (int ch = 0; ch < 24; ch++)
        ndspChnSetInterp(ch, S.interp);
    for (int ch = CH_LEAD; ch <= CH_HAT; ch++)
        apply_filter(ch);
    apply_filter(CH_WAV);
}

typedef struct { const char *name; float size, decay, damping, diffusion, width, level; } reverb_preset_t;
static const reverb_preset_t presets[] = {
    {"Off", 0, 0, 0, 0, 0, 0},
    {"Room", 0.35f, 0.8f, 0.5f, 0.6f, 0.8f, 0.8f},
    {"Hall", 0.9f, 2.6f, 0.35f, 0.75f, 1.0f, 0.7f},
    {"Plate", 0.6f, 1.6f, 0.15f, 0.9f, 1.0f, 0.8f},
    {"Cave", 1.0f, 6.0f, 0.6f, 0.5f, 1.0f, 0.6f},
};
static u32 caps;
static u16 firmware_version;

const char *audio_reverb_name(int p) { return p >= 0 && p < audio_reverb_count() ? presets[p].name : "?"; }
int audio_reverb_count(void) { return sizeof presets / sizeof presets[0]; }
float audio_reverb_default_decay(int p) { return presets[p].decay; }
bool audio_effects_available(void) { return (caps & OPENDSP_CAP_REVERB) && (caps & OPENDSP_CAP_DELAY); }
u16 audio_firmware_version(void) { return firmware_version; }

void audio_apply_effects(void)
{
    if (!audio_effects_available())
        return;
    if (S.reverb) {
        const reverb_preset_t *p = &presets[S.reverb];
        opendspReverbConfig r = {sizeof r, p->size, S.reverb_decay / 10.f, p->damping, p->diffusion, p->width, p->level};
        opendspReverbSet(0, &r);
    } else {
        opendspReverbSet(0, NULL);
    }
    ndspAuxSetVolume(0, S.reverb_return / 100.f);
    if (S.delay) {
        opendspDelayConfig d = {sizeof d, (float)S.delay_ms, S.delay_feedback / 100.f, 0.4f, 0.6f};
        opendspDelaySet(1, &d);
    } else {
        opendspDelaySet(1, NULL);
    }
    ndspAuxSetVolume(1, 1.0f);
    // sends changed: refresh every channel
    LightLock_Lock(&lock);
    for (int ch = 0; ch < 24; ch++)
        channels[ch].last_l = channels[ch].last_r = -1;
    LightLock_Unlock(&lock);
}

bool audio_init(char *error, size_t error_len)
{
    LightLock_Init(&lock);
    LightEvent_Init(&frame_event, RESET_ONESHOT);
    srand(0x0D5B);
    make_tables();
    make_drums();
    make_sfx();
    Result rc = opendspInit();
    if (R_FAILED(rc)) {
        snprintf(error, error_len, "opendspInit failed: 0x%08lX", rc);
        return false;
    }
    for (int ch = 0; ch < 24; ch++)
        ndspChnReset(ch);
    capture_buf = calloc(CAPTURE_FRAMES * 2, sizeof(s16));
    memset(&capture, 0, sizeof capture);
    capture.data_pcm16 = capture_buf;
    capture.nsamples = CAPTURE_FRAMES;
    ndspSetCapture(&capture);
    opendspGetCapabilities(&caps);
    opendspGetFirmwareVersion(&firmware_version);
    audio_apply_settings();
    audio_apply_effects();
    ndspSetCallback(on_frame, NULL);
    s32 priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    sequencer_running = true;
    sequencer = threadCreate(sequencer_main, NULL, 32 * 1024, priority - 2, -2, false);
    return true;
}

void audio_exit(void)
{
    sequencer_running = false;
    LightEvent_Signal(&frame_event);
    if (sequencer) {
        threadJoin(sequencer, U64_MAX);
        threadFree(sequencer);
        sequencer = NULL;
    }
    ndspSetCallback(NULL, NULL);
    ndspSetCapture(NULL);
    opendspExit();
    free(capture_buf);
}

void music_play(bool on)
{
    LightLock_Lock(&lock);
    if (on && !music_on) {
        bar = 0;
        step = 0;
        step_clock = -1.f;              // first step at once
    }
    if (!on)
        for (int ch = CH_LEAD; ch <= CH_BASS; ch++)
            tone_off(ch);
    music_on = on;
    LightLock_Unlock(&lock);
}

bool music_is_playing(void) { return music_on; }
int music_bpm(void) { return tempo; }
void music_set_bpm(int bpm) { tempo = bpm < 60 ? 60 : bpm > 200 ? 200 : bpm; }
bool music_muted(int track) { return muted[track]; }

void music_set_muted(int track, bool mute)
{
    LightLock_Lock(&lock);
    muted[track] = mute;
    if (mute && track <= 2)
        tone_off(CH_LEAD + track);
    LightLock_Unlock(&lock);
}

const char *music_track_name(int track) { return track_names[track]; }
const char *music_track_format(int track) { return track_formats[track]; }
int music_bar(void) { return bar; }
int music_step(void) { return step; }
const char *music_chord(void) { return chord_names[bar]; }

int sfx_count(void) { return FX_COUNT; }
const char *sfx_name(int i) { return sfx[i].name; }
const char *sfx_format(int i) { return sfx[i].format_name; }

static int sfx_next;

void sfx_play(int i)
{
    LightLock_Lock(&lock);
    int ch = -1;
    for (int k = 0; k < CH_NSFX; k++)
        if (!ndspChnIsPlaying(CH_SFX0 + k)) {
            ch = CH_SFX0 + k;
            break;
        }
    if (ch < 0) {
        ch = CH_SFX0 + sfx_next;
        sfx_next = (sfx_next + 1) % CH_NSFX;
    }
    play_sample(ch, &sfx[i], 1.0f, 1.0f, 0);
    LightLock_Unlock(&lock);
}

void ui_blip(int kind)
{
    if (!S.ui_sounds || kind < 0 || kind > 2)
        return;
    LightLock_Lock(&lock);
    play_sample(CH_UI, &blips[kind], 1.0f, 1.0f, 0);
    LightLock_Unlock(&lock);
}

void key_on(int slot, int midi)
{
    LightLock_Lock(&lock);
    tone_on(CH_KEY0 + slot, S.key_wave, note_hz(midi), 0.45f, 0, 2, 40, 0.55f, 40, -1, false);
    LightLock_Unlock(&lock);
}

void key_off(int slot)
{
    LightLock_Lock(&lock);
    tone_off(CH_KEY0 + slot);
    LightLock_Unlock(&lock);
}

// A minor 9 over four octaves, slightly detuned
void lab_chord(bool on)
{
    static const u8 notes[8] = {33, 40, 45, 48, 52, 55, 59, 62};
    LightLock_Lock(&lock);
    if (on) {
        music_on = false;
        formats_frame = -1;
        burst_left = 0;
        for (int ch = 0; ch < 24; ch++) {
            int n = notes[ch % 8] + (ch / 8) * 12;
            float detune = 1.0f + ((ch % 3) - 1) * 0.0025f;
            tone_on(ch, ch % 2 ? W_TRIANGLE : W_SINE, note_hz(n) * detune, 0.075f, ((ch % 7) - 3) / 3.5f,
                    120, 1, 1.0f, 160, -1, false);
        }
    } else {
        for (int ch = 0; ch < 24; ch++)
            tone_off(ch);
    }
    pad_on = on;
    LightLock_Unlock(&lock);
}

bool lab_chord_on(void) { return pad_on; }

void lab_burst(void)
{
    burst_wait = 0;
    burst_left = 36;
}

void lab_formats(void) { formats_frame = 0; }

void stop_all(void)
{
    LightLock_Lock(&lock);
    music_on = false;
    pad_on = false;
    burst_left = 0;
    formats_frame = -1;
    for (int ch = 0; ch < 24; ch++) {
        if (ch == CH_WAV)
            continue;
        channels[ch].stage = 0;
        channels[ch].running = false;
        ndspChnWaveBufClear(ch);
    }
    LightLock_Unlock(&lock);
}

u32 audio_frames(void) { return ndspGetFrameCount(); }
u32 audio_dropped(void) { return ndspGetDroppedFrames(); }

const s16 *audio_capture(u32 *position, u32 *length)
{
    *position = capture.offset;
    *length = CAPTURE_FRAMES;
    return capture_buf;
}
