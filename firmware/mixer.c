// SPDX-License-Identifier: BSD-2-Clause
// Mixing buses, aux, delay, output stage.
#include "mixer.h"
#include "shared_mem.h"
#include "source.h"
#include "dma.h"
#include "extension.h"
#include "reverb.h"
#include "surround.h"
#include "output.h"

#define F FRAME_SAMPLES
#define DELAY_MAX_FRAMES 200

static s32 bus[NUM_BUSES][2 * F];
static u16 bus_used[NUM_BUSES];
static s16 volume[3] = {0x4000, 0, 0}; // master, aux 0, aux 1 (Q14)
static u16 aux_enabled[2], aux_bypass[2], output_format = 1, clip_mode = 1;
static u16 rear_mode; // 0 fold, 1 rear ratio, 2 surround
// gains are read as the Y operand: Y memory
static mix_job_t mix_job YMEM_BSS;
static mix_ramp_job_t ramp_job;
static s16 ramp_gains[32] YMEM_BSS; // L R per step

// 16 ramp steps, from + d * s / 16 truncated, into g[0], g[2], ...
static void ramp_steps(s16 *g, s16 from, s16 to)
{
    u16 f = (u16)from, d, q, m, fraction = 0, s;
    if (to >= from) {
        d = (u16)to - f;
        q = d >> 4;
        m = d & 15;
        for (s = 16; s; s--) { // step s: f + q s + (m s >> 4)
            f += q;
            fraction += m;
            *g = (s16)(f + (fraction >> 4));
            g += 2;
        }
    } else {
        d = f - (u16)to;
        q = d >> 4;
        m = d & 15;
        for (s = 16; s; s--) {
            f -= q;
            fraction += m;
            *g = (s16)(f - (fraction >> 4));
            g += 2;
        }
    }
}

// feedback delay line, Q7 coefficients
typedef struct {
    u16 enabled, frames, position, channels;
    s16 g, a, b, pad;
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
    reverb_init();
    surround_init();
    output_format = 1;
    rear_mode = 0;
    clip_mode = 1;
}

// bit 0: front kept separate, bit 1: rear
u16 mixer_route(u16 mix)
{
    u16 b, route = 0;
    if (!mix)
        return 0;
    b = mix - 1;
    if ((aux_enabled[b] && !aux_bypass[b]) || delays[b].enabled || reverb_enabled(b))
        route |= 1;
    if (aux_enabled[b] || delays[b].enabled || reverb_enabled(b))
        route |= 2;
    return route;
}

s16 mixer_volume(u16 mix)
{
    return volume[mix];
}
u16 mixer_rear_mode(void)
{
    return rear_mode;
}

// plain assignments: teak-llvm miscompiles return x == y
static void update_rear_mode(void)
{
    rear_mode = 0;
    if (surround_rear_ratio() != 0x8000)
        rear_mode = 1;
    if (output_format == 2)
        rear_mode = 2;
}

static s16 read_volume(vu16 *f)
{
    s32 v = float_to_fixed(f[0], f[1], 14);
    return v < 0 ? 0 : v > 0x7FFF ? 0x7FFF : (s16)v;
}

u16 mixer_set_delay(u16 b, const delay_params_t *p)
{
    delay_t *e = &delays[b];
    u16 frames = p->frames;
    u32 address = p->address & ~3UL;
    if (!p->enable || !frames) {
        e->enabled = 0;
        return EXT_OK;
    }
    if (frames > DELAY_MAX_FRAMES)
        frames = DELAY_MAX_FRAMES;
    if (address < EXT_FCRAM_START || address + times1280(frames) > EXT_FCRAM_END) {
        e->enabled = 0;
        return EXT_ERR_ADDRESS;
    }
    e->frames = frames;
    e->channels = p->channels > 4 ? 4 : p->channels;
    e->g = p->g;
    e->a = p->a;
    e->b = p->b;
    if (!e->enabled || address != e->address) {
        u16 k;
        if (!p->zeroed) { // clear the delay line
            for (k = 0; k < 4 * F; k++)
                delay_buf[k] = 0;
            for (k = 0; k < e->frames; k++)
                dma_write((u16)(unsigned)delay_buf, 4 * F, address + times1280(k));
        }
        e->address = address;
        e->position = 0;
        e->last[0] = e->last[1] = e->last[2] = e->last[3] = 0;
    }
    e->enabled = 1;
    return EXT_OK;
}

static delay_params_t delay_params;

static void parse_delay(u16 b, vu16 *d)
{
    if (!d[0])
        return;
    d[0] = 0;
    if (d[3] > 4) // the original ignores such updates
        return;
    delay_params.enable = d[1];
    delay_params.channels = d[3];
    delay_params.frames = d[6];
    delay_params.g = (s16)d[7];
    delay_params.a = (s16)d[8];
    delay_params.b = (s16)d[9];
    delay_params.zeroed = 0;
    delay_params.address = ((u32)d[4] << 16) | d[5];
    mixer_set_delay(b, &delay_params);
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
    if ((hi & (1u << 10)) && output_format != d[11]) {
        output_format = d[11];
        changed = 1; // rear routing depends on it
    }
    if (hi & (1u << 11))
        clip_mode = d[12];
    if ((hi & 0xF000) && surround_parse(hi, d))
        changed = 1;
    if (changed)
        update_rear_mode();
    return changed;
}

void mixer_begin(void)
{
    u16 b;
    bus_used[0] = 1;
    for (b = 1; b < BUS_MAIN_REAR; b++)
        bus_used[b] = (mixer_route((b + 1) >> 1) >> ((b + 1) & 1)) & 1;
    bus_used[BUS_MAIN_REAR] = rear_mode != 0;
    bus_used[BUS_POST] = rear_mode == 2;
    for (b = 0; b < NUM_BUSES; b++)
        if (bus_used[b])
            clear_words(bus[b], 4 * F);
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
            output_poll();
            mix_add(&mix_job);
        } else { // ramp: 16 steps of 10 samples
            ramp_steps(&ramp_gains[0], pl, gl);
            ramp_steps(&ramp_gains[1], pr, gr);
            ramp_job.acc = bus[b];
            ramp_job.src = src;
            ramp_job.gains = ramp_gains;
            output_poll();
            mix_ramp(&ramp_job);
        }
    }
}

static u16 delay_job[4];
static s16 delay_coefs[3];
void delay_run(const u16 *job);

static void run_delay(delay_t *e, s32 **quad)
{
    u16 ch, n;
    u32 address = e->address + times1280(e->position);
    dma_read(address, 4 * F, (u16)(unsigned)delay_buf);
    delay_coefs[0] = e->b;
    delay_coefs[1] = e->a;
    delay_coefs[2] = e->g;
    delay_job[2] = (u16)(unsigned)delay_coefs;
    for (ch = 0; ch < e->channels; ch++) {
        if (!quad[ch])
            continue;
        delay_job[3] = (u16)(unsigned)&e->last[ch];
        for (n = 0; n < F; n += 80) {
            delay_job[0] = (u16)(unsigned)&quad[ch][2 * n];
            delay_job[1] = (u16)(unsigned)&delay_buf[ch * F + n];
            output_poll();
            delay_run(delay_job);
        }
    }
    dma_write((u16)(unsigned)delay_buf, 4 * F, address);
    if (++e->position >= e->frames)
        e->position = 0;
}

static s32 *quad[4]; // global: teak-llvm limit
static const s32 zero_bus[2 * F];
static u16 return_job[6];
static u16 output_job[5];
#define OUTPUT_CHUNK 32
void output_stage(const u16 *job);

void bus_normalize(s32 *x);
void bus_return_half(const u16 *job);

// both halves of a frame
static void bus_return(u16 *job)
{
    u16 k;
    output_poll();
    bus_return_half(job);
    for (k = 0; k < 5; k++)
        job[k] += 320; // 80 samples of s32 at stride 2
    output_poll();
    bus_return_half(job);
    for (k = 0; k < 5; k++)
        job[k] -= 320;
}

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
        for (ch = 0; ch < 4; ch++)
            if (quad[ch])
                bus_normalize(quad[ch]);
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
        if (reverb_enabled(b))
            reverb_run(b, quad);
        return_job[0] = (u16)(unsigned)(quad[0] ? quad[0] : zero_bus);
        return_job[1] = (u16)(unsigned)(quad[1] ? quad[1] : zero_bus);
        return_job[2] = (u16)(unsigned)quad[2];
        return_job[3] = (u16)(unsigned)quad[3];
        return_job[4] = (u16)(unsigned)bus[0];
        return_job[5] = (u16)volume[b + 1];
        if (rear_mode) { // rear stays rear, front after surround
            return_job[2] = return_job[3] = (u16)(unsigned)zero_bus;
            if (rear_mode == 2)
                return_job[4] = (u16)(unsigned)bus[BUS_POST];
            if (quad[0])
                bus_return(return_job);
            return_job[0] = return_job[1] = (u16)(unsigned)zero_bus;
            return_job[2] = (u16)(unsigned)quad[2];
            return_job[3] = (u16)(unsigned)quad[3];
            return_job[4] = (u16)(unsigned)bus[BUS_MAIN_REAR];
        }
        bus_return(return_job);
    }
    if (rear_mode)
        surround_run(bus[0], bus[BUS_MAIN_REAR], bus[BUS_POST], rear_mode);
    output_job[2] = 0;
    if (output_format == 0)
        output_job[2] = 1;
    output_job[3] = clip_mode;
    output_job[4] = OUTPUT_CHUNK;
    for (k = 0; k < F; k += OUTPUT_CHUNK) { // short spans with interrupts off
        output_job[0] = (u16)(unsigned)&bus[0][2 * k];
        output_job[1] = (u16)(unsigned)&out[2 * k];
        output_poll();
        output_stage(output_job);
    }
}
