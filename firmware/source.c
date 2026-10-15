// SPDX-License-Identifier: BSD-2-Clause
// Source configuration parsing, filters, mixing and status.
#include "source.h"
#include "shared_mem.h"
#include "mixer.h"

#define CONFIG_WORDS 96
#define STATUS_WORDS 6
#define SOURCE_CONFIG(i) ((vu16 *)(SHM_SOURCE_CONFIG + (i) * CONFIG_WORDS))
#define SOURCE_STATUS(i) ((vu16 *)(SHM_SOURCE_STATUS + (i) * STATUS_WORDS))
#define DSP_CONFIG       ((vu16 *)SHM_DSP_CONFIG)

// dirty flags, word 0
#define DIRTY_FORMAT        (1u << 0)
#define DIRTY_MONO          (1u << 1)
#define DIRTY_ADPCM_COEFS   (1u << 2)
#define DIRTY_PARTIAL_RESET (1u << 4)
// dirty flags, word 1
#define DIRTY_ENABLE        (1u << 0)
#define DIRTY_INTERP        (1u << 1)
#define DIRTY_RATE          (1u << 2)
#define DIRTY_QUEUE         (1u << 3)
#define DIRTY_PLAY_POSITION (1u << 5)
#define DIRTY_FILTER_ENABLE (1u << 6)
#define DIRTY_SIMPLE_FILTER (1u << 7)
#define DIRTY_BIQUAD_FILTER (1u << 8)
#define DIRTY_GAINS         (7u << 9)
#define DIRTY_SYNC          (1u << 12)
#define DIRTY_RESET         (1u << 13)
#define DIRTY_EMBEDDED      (1u << 14)

typedef struct {
    s16 gains[3][4];               // per mix: FL, FR, RL, RR (Q14)
    s16 bus_gain[NUM_BUSES][2];
    s16 previous[NUM_BUSES][2];
    u16 fresh;                     // no ramp on the next frame
    u16 filters;                   // bit 0 simple, bit 1 biquad
    s16 simple_coefs[2];
    s16 simple_state[2][2];
    s16 biquad_coefs[5];
    s16 biquad_state[2][5];
} source_mix_t;

static voice_t voices[NUM_SOURCES];
static source_mix_t mixes[NUM_SOURCES];
static u16 sync_count[NUM_SOURCES];
static u32 play_position[NUM_SOURCES];
static s16 voice_out[2 * FRAME_SAMPLES];
static wavebuf_t new_buffer;
static u16 gains_stale;

s32 float_to_fixed(u16 lo, u16 hi, u16 frac_bits)
{
    u16 exponent = (hi >> 7) & 0xFF;
    u32 mantissa = ((u32)(hi & 0x7F) << 16) | lo | 0x800000UL;
    s16 shift = (s16)exponent - 150 + (s16)frac_bits;
    s32 v;
    if (exponent == 0)
        return 0;
    if (lo == 0 && (hi & 0x7FFF) == 0x3F80)     // +-1.0
        return (hi & 0x8000) ? -((s32)1 << frac_bits) : ((s32)1 << frac_bits);
    if (shift >= 8)
        v = 0x7FFFFFFFL;
    else if (shift >= 0)
        v = (s32)(mantissa << shift);
    else if (shift > -24)
        v = (s32)(mantissa >> (u16)(-shift));
    else
        v = 0;
    return (hi & 0x8000) ? -v : v;
}

static s16 sat16(s32 v) { return v > 32767 ? 32767 : v < -32768 ? -32768 : (s16)v; }

static void reset_filters(source_mix_t *m, u16 which)
{
    u16 c;
    if (which & 1) {
        m->simple_coefs[0] = 0x7FFF;
        m->simple_coefs[1] = 0;
        for (c = 0; c < 2; c++)
            m->simple_state[c][0] = m->simple_state[c][1] = 0;
    }
    if (which & 2) {
        m->biquad_coefs[0] = 0x4000;
        m->biquad_coefs[1] = m->biquad_coefs[2] = m->biquad_coefs[3] = m->biquad_coefs[4] = 0;
        for (c = 0; c < 2; c++)
            m->biquad_state[c][0] = m->biquad_state[c][1] = m->biquad_state[c][2] =
                m->biquad_state[c][3] = m->biquad_state[c][4] = 0;
    }
}

static void reset_mix(u16 i)
{
    volatile u16 *p = (volatile u16 *)&mixes[i];
    u16 k;
    for (k = 0; k < sizeof(source_mix_t) / sizeof(u16); k++)
        p[k] = 0;
    mixes[i].fresh = 1;
    reset_filters(&mixes[i], 3);
}

void sources_init(void)
{
    u16 i;
    for (i = 0; i < NUM_SOURCES; i++) {
        voices[i].coef_slot = i;
        voice_reset(&voices[i]);
        reset_mix(i);
        sync_count[i] = 0;
        play_position[i] = 0;
    }
    mixer_init();
    gains_stale = 1;
}

static void read_u32(vu16 *p, u32 *out) { *out = ((u32)p[0] << 16) | p[1]; }

static void parse_gains(u16 i, vu16 *config)
{
    u16 mix, ch;
    for (mix = 0; mix < 3; mix++)
        for (ch = 0; ch < 4; ch++) {
            vu16 *g = config + 2 + 8 * mix + 2 * ch;
            mixes[i].gains[mix][ch] = sat16(float_to_fixed(g[0], g[1], 14));
        }
}

// fold source gains into bus gains
static void update_gains(u16 i)
{
    source_mix_t *m = &mixes[i];
    s32 l = 0, r = 0;
    u16 mix, bus;
    for (bus = 0; bus < NUM_BUSES; bus++)
        m->bus_gain[bus][0] = m->bus_gain[bus][1] = 0;
    for (mix = 0; mix < 3; mix++) {
        const s16 *g = m->gains[mix];
        u16 route = mixer_route(mix);
        s16 volume = mixer_volume(mix);
        if (!(route & 1)) {
            l += mul16(g[0], volume);
            r += mul16(g[1], volume);
        } else {
            m->bus_gain[1 + 2 * (mix - 1)][0] = g[0];
            m->bus_gain[1 + 2 * (mix - 1)][1] = g[1];
        }
        if (!(route & 2)) {
            l += mul16(g[2], volume);
            r += mul16(g[3], volume);
        } else {
            m->bus_gain[2 + 2 * (mix - 1)][0] = g[2];
            m->bus_gain[2 + 2 * (mix - 1)][1] = g[3];
        }
    }
    m->bus_gain[0][0] = sat16(l >> 14);
    m->bus_gain[0][1] = sat16(r >> 14);
}

static void parse_source(u16 i)
{
    vu16 *c = SOURCE_CONFIG(i);
    voice_t *v = &voices[i];
    source_mix_t *m = &mixes[i];
    u16 lo = c[0], hi = c[1];
    if (!lo && !hi)
        return;
    c[0] = 0;
    c[1] = 0;
    if (hi & DIRTY_RESET) {
        voice_reset(v);
        play_position[i] = 0;
        m->fresh = 1;
        m->filters = 0;
        reset_filters(m, 3);
    }
    if (lo & DIRTY_PARTIAL_RESET) {     // drop queue and current buffer
        v->queued = 0;
        v->primed = 0;
        v->has_current = 0;
        v->frac = 0;
    }
    if (hi & DIRTY_SYNC)
        sync_count[i] = c[81];
    if (hi & DIRTY_RATE) {
        s32 rate = float_to_fixed(c[26], c[27], 16);
        v->step = rate < 0 ? 0 : (u32)rate;
    }
    if (hi & DIRTY_INTERP)
        v->interp = c[28] & 0xFF;
    if (hi & DIRTY_SIMPLE_FILTER) {
        m->simple_coefs[0] = (s16)c[30];
        m->simple_coefs[1] = (s16)c[31];
    }
    if (hi & DIRTY_BIQUAD_FILTER) {     // stored a2, a1, b2, b1, b0
        m->biquad_coefs[0] = (s16)c[36];
        m->biquad_coefs[1] = (s16)c[35];
        m->biquad_coefs[2] = (s16)c[34];
        m->biquad_coefs[3] = (s16)c[33];
        m->biquad_coefs[4] = (s16)c[32];
    }
    if (hi & DIRTY_FILTER_ENABLE) {
        u16 on = c[29] & 3, off = (u16)(m->filters & ~on);
        if (off & 1)
            m->simple_state[0][1] = m->simple_state[1][1] = 0;
        if (off & 2) {
            u16 ch, k;
            for (ch = 0; ch < 2; ch++)
                for (k = 0; k < 5; k++)
                    m->biquad_state[ch][k] = 0;
        }
        m->filters = on;
    }
    if (hi & DIRTY_GAINS) {
        parse_gains(i, c);
        update_gains(i);
    }
    if (lo & DIRTY_ADPCM_COEFS) {
        vu16 *k = (vu16 *)(SHM_ADPCM_COEFS + (i << 4));
        u16 j;
        for (j = 0; j < 16; j++)
            v->coefs[j] = (s16)k[j];
        v->coef_version++;
    }
    if (hi & DIRTY_PLAY_POSITION)
        read_u32(c + 82, &play_position[i]);
    if ((lo & (DIRTY_FORMAT | DIRTY_MONO)) || (hi & DIRTY_EMBEDDED)) {
        u16 f = c[90];
        v->stereo = ((f & 3) == 2) ? 1 : 0;
        v->format = (f >> 2) & 3;
    }
    if (hi & DIRTY_EMBEDDED) {
        read_u32(c + 86, &new_buffer.address);
        read_u32(c + 88, &new_buffer.length);
        new_buffer.start = play_position[i];
        play_position[i] = 0;
        new_buffer.id = c[95];
        new_buffer.looping = (c[94] >> 1) & 1;
        new_buffer.adpcm_context = c[94] & 1;
        new_buffer.yn1 = (s16)c[92];
        new_buffer.yn2 = (s16)c[93];
        voice_queue(v, &new_buffer);
    }
    if (hi & DIRTY_QUEUE) {
        u16 mask = c[37], k;
        c[37] = 0;
        for (k = 0; k < 4; k++) {
            vu16 *b = c + 38 + 10 * k;
            if (!(mask & (1u << k)))
                continue;
            read_u32(b, &new_buffer.address);
            read_u32(b + 2, &new_buffer.length);
            new_buffer.start = 0;
            new_buffer.id = b[8];
            new_buffer.looping = (b[7] >> 8) & 1;
            new_buffer.adpcm_context = b[7] & 0xFF;
            new_buffer.yn1 = (s16)b[5];
            new_buffer.yn2 = (s16)b[6];
            voice_queue(v, &new_buffer);
        }
    }
    if (hi & DIRTY_ENABLE) {
        u16 on = c[80] & 0xFF;
        if (on && !v->enabled) {
            v->primed = 0;
            m->fresh = 1;
        }
        v->enabled = on;
    }
}

static filter_job_t filter_job;
static s16 filter_coefs[5] YMEM;

static void run_filters(source_mix_t *m)
{
    u16 ch, k;
    if (m->filters & 1) {
        filter_coefs[0] = m->simple_coefs[0];
        filter_coefs[1] = m->simple_coefs[1];
        for (ch = 0; ch < 2; ch++) {
            filter_job.buf = voice_out + ch;
            filter_job.state = m->simple_state[ch];
            filter_job.coefs = filter_coefs;
            filter_simple(&filter_job);
        }
    }
    if (m->filters & 2) {
        for (k = 0; k < 5; k++)
            filter_coefs[k] = m->biquad_coefs[k];
        for (ch = 0; ch < 2; ch++) {
            filter_job.buf = voice_out + ch;
            filter_job.state = m->biquad_state[ch];
            filter_job.coefs = filter_coefs;
            filter_biquad(&filter_job);
        }
    }
}

void sources_render(u16 read_bank, u16 write_bank, vu16 *out)
{
    u16 i;
    select_bank(read_bank);
    for (i = 0; i < NUM_SOURCES; i++)
        parse_source(i);
    if (mixer_parse(DSP_CONFIG))
        gains_stale = 1;
    select_bank(0);

    mixer_begin();
    for (i = 0; i < NUM_SOURCES; i++) {
        voice_t *v = &voices[i];
        source_mix_t *m = &mixes[i];
        if (gains_stale)
            update_gains(i);
        if (!v->enabled) {
            m->fresh = 1;
            continue;
        }
        voice_render(v, voice_out);
        if (m->filters)
            run_filters(m);
        mixer_add_source(voice_out, m->bus_gain, m->previous, m->fresh);
        m->fresh = 0;
    }
    gains_stale = 0;
    mixer_end(read_bank, write_bank, out);

    select_bank(write_bank);
    for (i = 0; i < NUM_SOURCES; i++) {
        voice_t *v = &voices[i];
        vu16 *st = SOURCE_STATUS(i);
        st[0] = (v->enabled ? 1 : 0) | (v->id_changed ? 0x100 : 0);
        st[1] = sync_count[i];
        st[2] = (u16)(v->status_position >> 16);
        st[3] = (u16)v->status_position;
        st[4] = v->current_id;
        st[5] = v->last_id;
        v->id_changed = 0;
    }
    select_bank(0);
}
