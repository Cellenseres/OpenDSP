// SPDX-License-Identifier: BSD-2-Clause
// Mixing buses, aux, delay, output stage.
#include "mixer.h"
#include "shared_mem.h"
#include "source.h"
#include "dma.h"

#define F FRAME_SAMPLES
#define DELAY_MAX_FRAMES 200

static s32 bus[NUM_BUSES][2 * F];
static u16 bus_used[NUM_BUSES];
static s16 volume[3] = {0x4000, 0, 0}; // master, aux 0, aux 1 (Q14)
static u16 aux_enabled[2], aux_bypass[2], output_format = 1, clip_mode = 1;
static mix_job_t mix_job;

// feedback delay line, Q7 coefficients
typedef struct {
    u16 enabled, frames, position;
    s16 g, a, b;
    u32 address;
    s16 last[4];
} delay_t;
static delay_t delays[2];
static s16 delay_buf[4 * F] __attribute__((aligned(4)));

// teak-llvm lacks i32 MUL; opaque shifts stop folding
static volatile u16 shift8 = 8, shift10 = 10;
static u32 times1280(u32 x)
{
    return (x << shift10) + (x << shift8);
}

static s16 sat16(s32 v)
{
    return v > 32767 ? 32767 : v < -32768 ? -32768 : (s16)v;
}

void mixer_init(void)
{
    u16 b;
    volume[0] = 0x4000;
    volume[1] = volume[2] = 0;
    for (b = 0; b < 2; b++) {
        aux_enabled[b] = 0;
        aux_bypass[b] = 0;
        delays[b].enabled = 0;
    }
    output_format = 1;
    clip_mode = 1;
}

// bit 0: front kept separate, bit 1: rear
u16 mixer_route(u16 mix)
{
    u16 b, route = 0;
    if (!mix)
        return 0;
    b = mix - 1;
    if ((aux_enabled[b] && !aux_bypass[b]) || delays[b].enabled)
        route |= 1;
    if (aux_enabled[b] || delays[b].enabled)
        route |= 2;
    return route;
}

s16 mixer_volume(u16 mix)
{
    return volume[mix];
}

static s16 read_volume(vu16 *f)
{
    s32 v = float_to_fixed(f[0], f[1], 14);
    return v < 0 ? 0 : v > 0x7FFF ? 0x7FFF : (s16)v;
}

static void parse_delay(u16 b, vu16 *d)
{
    delay_t *e = &delays[b];
    if (!d[0])
        return;
    d[0] = 0;
    if (!d[1] || !d[6]) {
        e->enabled = 0;
        return;
    }
    e->frames = d[6] > DELAY_MAX_FRAMES ? DELAY_MAX_FRAMES : d[6];
    e->g = (s16)d[7];
    e->a = (s16)d[8];
    e->b = (s16)d[9];
    {
        u32 address = (((u32)d[4] << 16) | d[5]) & ~3UL;
        if (!e->enabled || address != e->address) { // clear the delay line
            u16 k;
            for (k = 0; k < 4 * F; k++)
                delay_buf[k] = 0;
            for (k = 0; k < e->frames; k++)
                dma_write((u16)(unsigned)delay_buf, 4 * F, address + times1280(k));
            e->address = address;
            e->position = 0;
            e->last[0] = e->last[1] = e->last[2] = e->last[3] = 0;
        }
    }
    e->enabled = 1;
}

u16 mixer_parse(vu16 *d)
{
    u16 lo = d[0], hi = d[1], changed = 0, b;
    if (!lo && !hi)
        return 0;
    d[0] = 0;
    d[1] = 0;
    if (hi & 1u) {
        volume[0] = read_volume(d + 2);
        changed = 1;
    }
    for (b = 0; b < 2; b++) {
        if (hi & (1u << (8 + b))) {
            volume[1 + b] = read_volume(d + 4 + 2 * b);
            changed = 1;
        }
        if (lo & (1u << (8 + b))) {
            aux_enabled[b] = d[20 + b] != 0;
            changed = 1;
        }
        if (lo & (1u << (6 + b))) {
            aux_bypass[b] = d[18 + b] != 0;
            changed = 1;
        }
        if (lo & (1u << (10 + b))) {
            u16 was = delays[b].enabled;
            parse_delay(b, d + 22 + 10 * b);
            if (was != delays[b].enabled)
                changed = 1;
        }
    }
    if (hi & (1u << 10))
        output_format = d[11];
    if (hi & (1u << 11))
        clip_mode = d[12];
    return changed;
}

void mixer_begin(void)
{
    u16 b, k;
    bus_used[0] = 1;
    for (b = 1; b < NUM_BUSES; b++)
        bus_used[b] = (mixer_route((b + 1) >> 1) >> ((b + 1) & 1)) & 1;
    for (b = 0; b < NUM_BUSES; b++) {
        s32 *acc = bus[b];
        if (!bus_used[b])
            continue;
        for (k = 0; k < 2 * F; k++)
            acc[k] = 0;
    }
}

void mixer_add_source(const s16 *src, s16 gain[NUM_BUSES][2], s16 previous[NUM_BUSES][2], u16 fresh)
{
    u16 b;
    for (b = 0; b < NUM_BUSES; b++) {
        s16 gl = gain[b][0], gr = gain[b][1], pl = previous[b][0], pr = previous[b][1];
        previous[b][0] = gl;
        previous[b][1] = gr;
        if (!bus_used[b])
            continue;
        if (fresh) {
            pl = gl;
            pr = gr;
        }
        if (!gl && !gr && !pl && !pr)
            continue;
        mix_job.acc = bus[b];
        mix_job.src = src;
        if (pl == gl && pr == gr) {
            mix_job.gain_l = (u16)gl;
            mix_job.gain_r = (u16)gr;
            mix_job.count = F;
            mix_add(&mix_job);
        } else { // ramp: 16 steps of 10 samples
            s32 dl = (s32)gl - pl, dr = (s32)gr - pr, al = 0, ar = 0;
            u16 s;
            mix_job.count = 10;
            for (s = 0; s < 16; s++) {
                al += dl;
                ar += dr;
                // no >> on negative s32 (teak-llvm)
                mix_job.gain_l = (u16)(s16)(al < 0 ? pl - (s16)((u32)(-al) >> 4) : pl + (s16)((u32)al >> 4));
                mix_job.gain_r = (u16)(s16)(ar < 0 ? pr - (s16)((u32)(-ar) >> 4) : pr + (s16)((u32)ar >> 4));
                mix_add(&mix_job);
                mix_job.acc += 20;
                mix_job.src += 20;
            }
        }
    }
}

static void run_delay(delay_t *e, s32 **quad)
{
    u16 ch, n;
    u32 address = e->address + times1280(e->position);
    dma_read(address, 4 * F, (u16)(unsigned)delay_buf);
    for (ch = 0; ch < 4; ch++) {
        s32 *x = quad[ch];
        s16 *v = &delay_buf[ch * F], y = e->last[ch];
        if (!x)
            continue;
        for (n = 0; n < F; n++) {
            s16 in = sat16(x[2 * n]);
            y = sat16((mul16(e->b, y) + mul16(e->a, v[n])) >> 7);
            v[n] = sat16((s32)in - (mul16(e->g, y) >> 7));
            x[2 * n] = y;
        }
        e->last[ch] = y;
    }
    dma_write((u16)(unsigned)delay_buf, 4 * F, address);
    if (++e->position >= e->frames)
        e->position = 0;
}

static s32 *quad[4]; // global: teak-llvm limit

// s32 per sample, [mix][channel][160]
#define INTERMEDIATE_MIX(b, ch) ((vu16 *)(SHM_INTERMEDIATE_MIX + (b) * (8 * F) + (ch) * (2 * F)))

void mixer_end(u16 read_bank, u16 write_bank, vu16 *out)
{
    u16 b, k, ch;
    for (b = 0; b < 2; b++) {
        u16 route = mixer_route(b + 1);
        if (!route)
            continue;
        quad[0] = (route & 1) ? &bus[1 + 2 * b][0] : 0;
        quad[1] = (route & 1) ? &bus[1 + 2 * b][1] : 0;
        quad[2] = &bus[2 + 2 * b][0];
        quad[3] = &bus[2 + 2 * b][1];
        for (ch = 0; ch < 4; ch++) {
            s32 *x = quad[ch];
            if (!x)
                continue;
            for (k = 0; k < F; k++)
                x[2 * k] >>= 14;
        }
        if (aux_enabled[b]) {
            select_bank(write_bank);
            for (ch = 0; ch < 4; ch++) {
                vu16 *mix = INTERMEDIATE_MIX(b, ch);
                s32 *x = quad[ch];
                for (k = 0; k < F; k++) {
                    s32 v = x ? x[2 * k] : 0;
                    mix[2 * k] = (u16)v;
                    mix[2 * k + 1] = (u16)(v >> 16);
                }
            }
            select_bank(read_bank);
            for (ch = 0; ch < 4; ch++) {
                vu16 *mix = INTERMEDIATE_MIX(b, ch);
                s32 *x = quad[ch];
                if (!x)
                    continue;
                for (k = 0; k < F; k++)
                    x[2 * k] = (s32)(((u32)mix[2 * k + 1] << 16) | mix[2 * k]);
            }
            select_bank(0);
        }
        if (delays[b].enabled)
            run_delay(&delays[b], quad);
        for (k = 0; k < F; k++) {
            s32 l = quad[2][2 * k], r = quad[3][2 * k];
            if (quad[0]) {
                l += quad[0][2 * k];
                r += quad[1][2 * k];
            }
            bus[0][2 * k] += mul16(sat16(l), volume[b + 1]);
            bus[0][2 * k + 1] += mul16(sat16(r), volume[b + 1]);
        }
    }
    for (k = 0; k < F; k++) {
        s32 l = bus[0][2 * k] >> 14, r = bus[0][2 * k + 1] >> 14;
        if (output_format == 0) { // mono
            l = (l + r) >> 1;
            r = l;
        }
        if (clip_mode) { // soft knee above 0.75
            if (l > 24576)
                l = 24576 + ((l - 24576) >> 2);
            else if (l < -24576)
                l = -24576 + ((l + 24576) >> 2);
            if (r > 24576)
                r = 24576 + ((r - 24576) >> 2);
            else if (r < -24576)
                r = -24576 + ((r + 24576) >> 2);
        }
        out[2 * k] = (u16)sat16(l);
        out[2 * k + 1] = (u16)sat16(r);
    }
}
