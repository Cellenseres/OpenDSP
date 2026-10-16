// SPDX-License-Identifier: BSD-2-Clause
// Reverb: input diffusers, then a four line FDN with an allpass in each loop.
#include "reverb.h"
#include "voice.h"
#include "extension.h"
#include "output.h"

#define F FRAME_SAMPLES
#define DIFFUSERS 4
#define LINE_WORDS (1777 + 1543 + 1291 + 1009)
#define DIFFUSER_LENGTHS { 131, 199, 271, 359 }
#define DIFFUSER_WORDS (131 + 199 + 271 + 359)
#define LOOP_LENGTHS { 79, 97, 113, 151 }
#define LOOP_WORDS (79 + 97 + 113 + 151)
#define LOOP_GAIN 19661                 // 0.6
#define GAIN_MAX 32440                  // 0.99: the tail always decays
#define TANK_CHUNK 20                   // 20 x 124 instructions: below one sample period
// silent input, tail within +-QUIET_PEAK for longer than the longest line:
// clear the network and skip it until input returns
#define QUIET_PEAK 2
#define QUIET_FRAMES 16

// kernel table: dither, damping, lp0, g0, lp2, g2, lp1, g1, lp3, g3, same, cross
enum { T_DITHER, T_DAMPING, T_LP0, T_G0, T_LP2, T_G2, T_LP1, T_G1, T_LP3, T_G3, T_SAME, T_CROSS, T_WORDS };
#define DITHER_SEED 0xACE1
static const u16 lowpass_slot[REVERB_LINES] = { T_LP0, T_LP1, T_LP2, T_LP3 };

typedef struct {
    u16 enabled, idle, quiet;
    u16 length[REVERB_LINES];
    u16 position[REVERB_LINES];
    u16 diffuser_position[DIFFUSERS];
    u16 loop_position[REVERB_LINES];
    s16 diffusion, input_gain;
    s16 table[T_WORDS];
} reverb_t;

// delay lines in Y, away from the shared banks
static reverb_t reverbs[2];
static s16 lines[2][LINE_WORDS] YMEM_BSS;
static s16 diffusers[2][DIFFUSER_WORDS] YMEM_BSS;
static s16 loops[2][LOOP_WORDS] YMEM_BSS;
static const u16 line_max[REVERB_LINES] = EXT_REVERB_LINE_MAX;
static const u16 diffuser_length[DIFFUSERS] = DIFFUSER_LENGTHS;
static const u16 loop_length[REVERB_LINES] = LOOP_LENGTHS;
// buffer refs for the kernels: word addresses on the DSP, where teak-llvm
// pointer math is slow; plain pointers on the host
#ifdef __teak__
typedef u16 ref_t;
#define REF(p) ((u16)(unsigned)(p))
#else
typedef s16 *ref_t;
#define REF(p) (p)
#endif
static ref_t line_base[2][REVERB_LINES];
static ref_t diffuser_base[2][DIFFUSERS];
static ref_t loop_base[2][REVERB_LINES];

static s16 mono[F];
static s16 stereo[2 * F];

#ifdef __teak__
void reverb_input(const u16 *job);
void reverb_diffuse(const u16 *job);
void reverb_tank(const u16 *job);
void reverb_output(const u16 *job);
u16 reverb_or(const u16 *job);
static u16 job[8];
#else
// C versions of reverb_kernels.s with the same rounding
typedef long long s64;
static s16 sat16(s64 v) { return v > 32767 ? 32767 : v < -32768 ? -32768 : (s16)v; }
// value in 1/65536 units plus a fraction, floored, as in the kernels
static s16 quantize(s64 scaled, s64 fraction) { return sat16((scaled + fraction) >> 16); }
static u16 xorshift(u16 x) { x ^= x << 7; x ^= x >> 9; x ^= x << 8; return x; }

static void diffuse_ref(s16 *x, s16 *line, u16 count, s16 g)
{
    u16 n;
    for (n = 0; n < count; n++) {
        s16 d = line[n];
        s16 w = quantize(((s64)x[n] << 16) + 2 * (s64)g * d, 0x8000);
        s16 y = quantize(((s64)d << 16) - 2 * (s64)g * w, 0x8000);
        line[n] = w;
        x[n] = y;
    }
}

static void tank_ref(const s16 *in, s16 *out, s16 **ptr, u16 count, s16 *t)
{
    static const u16 gain_slot[REVERB_LINES] = { T_G0, T_G1, T_G2, T_G3 };
    u16 n, k;
    for (n = 0; n < count; n++) {
        s64 sum = 0, rnd;
        s16 left, right;
        t[T_DITHER] = (s16)xorshift((u16)t[T_DITHER]);
        rnd = (u16)t[T_DITHER];
        for (k = 0; k < REVERB_LINES; k++) {
            s16 out_k = ptr[k][n], *lp = &t[lowpass_slot[k]];
            s16 diff = sat16((s64)*lp - out_k);
            *lp = quantize(((s64)out_k << 16) + 2 * (s64)t[T_DAMPING] * diff, rnd);
            sum += *lp;
        }
        for (k = 0; k < REVERB_LINES; k++) {
            s16 fb = quantize((2 * (s64)t[lowpass_slot[k]] - sum) << 15, rnd);
            ptr[k][n] = quantize(((s64)in[n] << 16) + 2 * (s64)t[gain_slot[k]] * fb, rnd);
        }
        left = quantize(((s64)t[T_LP0] + t[T_LP2]) << 15, rnd);
        right = quantize(((s64)t[T_LP1] + t[T_LP3]) << 15, rnd);
        out[2 * n] = quantize(2 * ((s64)t[T_SAME] * left + (s64)t[T_CROSS] * right), rnd);
        out[2 * n + 1] = quantize(2 * ((s64)t[T_SAME] * right + (s64)t[T_CROSS] * left), rnd);
    }
}
#endif

static void clear(s16 *p, u16 words)
{
#ifdef __teak__
    clear_words(p, words);
#else
    while (words--)
        *p++ = 0;
#endif
}

static void clear_bus(u16 bus)
{
    reverb_t *r = &reverbs[bus];
    u16 k;
    clear(lines[bus], LINE_WORDS);
    clear(diffusers[bus], DIFFUSER_WORDS);
    clear(loops[bus], LOOP_WORDS);
    for (k = 0; k < REVERB_LINES; k++) {
        r->position[k] = 0;
        r->loop_position[k] = 0;
        r->table[lowpass_slot[k]] = 0;
    }
    for (k = 0; k < DIFFUSERS; k++)
        r->diffuser_position[k] = 0;
    r->table[T_DITHER] = DITHER_SEED;
    r->idle = 1;
    r->quiet = 0;
}

void reverb_init(void)
{
    u16 bus, k, offset;
    for (bus = 0; bus < 2; bus++) {
        reverbs[bus].enabled = 0;
        offset = 0;
        for (k = 0; k < REVERB_LINES; k++) {
            line_base[bus][k] = REF(&lines[bus][offset]);
            offset += line_max[k];
        }
        offset = 0;
        for (k = 0; k < DIFFUSERS; k++) {
            diffuser_base[bus][k] = REF(&diffusers[bus][offset]);
            offset += diffuser_length[k];
        }
        offset = 0;
        for (k = 0; k < REVERB_LINES; k++) {
            loop_base[bus][k] = REF(&loops[bus][offset]);
            offset += loop_length[k];
        }
    }
}

u16 reverb_set(u16 bus, const reverb_params_t *p)
{
    reverb_t *r = &reverbs[bus];
    u16 k, resized = 0;
    if (!p->enable) {
        r->enabled = 0;
        return EXT_OK;
    }
    for (k = 0; k < REVERB_LINES; k++)
        if (p->length[k] < EXT_REVERB_LINE_MIN || p->length[k] > line_max[k] || p->gain[k] < 0)
            return EXT_ERR_ARGUMENT;
    for (k = 0; k < REVERB_LINES; k++)
        if (r->length[k] != p->length[k])
            resized = 1;
    if (!r->enabled || resized) {
        for (k = 0; k < REVERB_LINES; k++)
            r->length[k] = p->length[k];
        clear_bus(bus);
    }
    r->table[T_G0] = p->gain[0] > GAIN_MAX ? GAIN_MAX : p->gain[0];
    r->table[T_G1] = p->gain[1] > GAIN_MAX ? GAIN_MAX : p->gain[1];
    r->table[T_G2] = p->gain[2] > GAIN_MAX ? GAIN_MAX : p->gain[2];
    r->table[T_G3] = p->gain[3] > GAIN_MAX ? GAIN_MAX : p->gain[3];
    r->table[T_DAMPING] = p->damping < 0 ? 0 : p->damping;
    r->table[T_SAME] = p->out_same;
    r->table[T_CROSS] = p->out_cross;
    r->diffusion = p->diffusion;
    r->input_gain = p->input_gain < 0 ? 0 : p->input_gain;
    r->enabled = 1;
    return EXT_OK;
}

u16 reverb_enabled(u16 bus) { return reverbs[bus].enabled; }

// allpass over samples[0, total); returns the new ring position
static u16 allpass(ref_t samples, u16 total, ref_t ring, u16 length, u16 pos, s16 gain)
{
    while (total) {
        u16 count = length - pos;
        if (count > total)
            count = total;
#ifdef __teak__
        job[0] = samples;
        job[1] = ring + pos;
        job[2] = count;
        job[3] = (u16)gain;
        output_poll();
        reverb_diffuse(job);
#else
        diffuse_ref(samples, ring + pos, count, gain);
#endif
        samples += count;
        total -= count;
        pos += count;
        if (pos == length)
            pos = 0;
    }
    return pos;
}

// scalars: teak-llvm spills indexed accesses as 32-bit values
static ref_t base0, base1, base2, base3;
static u16 pos0, pos1, pos2, pos3, len0, len1, len2, len3;
#ifndef __teak__
static s16 *segment_line[REVERB_LINES];
#endif

static reverb_t *tank_r;                // at most 4 arguments with teak-llvm
static u16 tank_bus;

// loop allpass over the frame just written to line k
static void loop_allpass(u16 k, ref_t base, u16 end, u16 len)
{
    u16 start = end >= F ? end - F : end + len - F, first = len - start, pos = tank_r->loop_position[k];
    ref_t ring = loop_base[tank_bus][k];
    if (first > F)
        first = F;
    pos = allpass(base + start, first, ring, loop_length[k], pos, LOOP_GAIN);
    if (first < F)
        pos = allpass(base, F - first, ring, loop_length[k], pos, LOOP_GAIN);
    tank_r->loop_position[k] = pos;
}

static void run_tank(reverb_t *r, u16 bus)
{
    u16 done = 0;
    pos0 = r->position[0];
    pos1 = r->position[1];
    pos2 = r->position[2];
    pos3 = r->position[3];
    len0 = r->length[0];
    len1 = r->length[1];
    len2 = r->length[2];
    len3 = r->length[3];
    base0 = line_base[bus][0];
    base1 = line_base[bus][1];
    base2 = line_base[bus][2];
    base3 = line_base[bus][3];
#ifdef __teak__
    job[7] = REF(r->table);
#endif
    while (done < F) {
        u16 count = F - done, room;
        if (count > TANK_CHUNK)         // short spans with interrupts off
            count = TANK_CHUNK;
        room = len0 - pos0;
        if (count > room)
            count = room;
        room = len1 - pos1;
        if (count > room)
            count = room;
        room = len2 - pos2;
        if (count > room)
            count = room;
        room = len3 - pos3;
        if (count > room)
            count = room;
#ifdef __teak__
        job[0] = REF(mono) + done;
        job[1] = REF(stereo) + 2 * done;
        job[2] = base0 + pos0;
        job[3] = base2 + pos2;
        job[4] = base1 + pos1;
        job[5] = base3 + pos3;
        job[6] = count;
        output_poll();
        reverb_tank(job);
#else
        segment_line[0] = base0 + pos0;
        segment_line[1] = base1 + pos1;
        segment_line[2] = base2 + pos2;
        segment_line[3] = base3 + pos3;
        tank_ref(&mono[done], &stereo[2 * done], segment_line, count, r->table);
#endif
        pos0 += count;
        if (pos0 == len0)
            pos0 = 0;
        pos1 += count;
        if (pos1 == len1)
            pos1 = 0;
        pos2 += count;
        if (pos2 == len2)
            pos2 = 0;
        pos3 += count;
        if (pos3 == len3)
            pos3 = 0;
        done += count;
    }
    r->position[0] = pos0;
    r->position[1] = pos1;
    r->position[2] = pos2;
    r->position[3] = pos3;
    // lines are longer than a frame: nothing written here is read yet
    tank_r = r;
    tank_bus = bus;
    loop_allpass(0, base0, pos0, len0);
    loop_allpass(1, base1, pos1, len1);
    loop_allpass(2, base2, pos2, len2);
    loop_allpass(3, base3, pos3, len3);
}

static u16 all_zero(const s16 *x, u16 count)
{
#ifdef __teak__
    u16 bits;
    if (x[0])                           // the common case
        return 0;
    job[0] = REF(x);
    job[1] = count;
    bits = reverb_or(job);
    if (bits)
        return 0;
    return 1;
#else
    const s16 *end = x + count;
    while (x != end)
        if (*x++)
            return 0;
    return 1;
#endif
}

// any sample outside +-QUIET_PEAK
static u16 any_loud(const s16 *x, u16 count)
{
    const s16 *end = x + count;
    while (x != end) {
        s16 v = *x++;
        if (v > QUIET_PEAK || v < -QUIET_PEAK)
            return 1;
    }
    return 0;
}

// diffusers and tank on mono, or silence while idle
static void run_network(reverb_t *r, u16 bus)
{
    u16 d, silent = all_zero(mono, F), loud;
    if (r->idle && silent) {
        for (d = 0; d < 2 * F; d++)
            stereo[d] = 0;
        return;
    }
    r->idle = 0;
    for (d = 0; d < DIFFUSERS; d++)
        r->diffuser_position[d] = allpass(REF(mono), F, diffuser_base[bus][d], diffuser_length[d],
                                          r->diffuser_position[d], r->diffusion);
    run_tank(r, bus);
    loud = !silent || any_loud(stereo, 2 * F);
    if (loud) {
        r->quiet = 0;
        return;
    }
    if (++r->quiet >= QUIET_FRAMES)
        clear_bus(bus);
}

void reverb_run(u16 bus, s32 **quad)
{
    reverb_t *r = &reverbs[bus];
#ifdef __teak__
    u16 d;
    u16 k;
    for (k = 0; k < 4; k++)
        job[k] = (u16)(unsigned)quad[k];
    job[5] = (u16)r->input_gain;
    for (k = 0; k < F; k += F / 2) {    // halves: short spans with interrupts off
        job[4] = (u16)(unsigned)&mono[k];
        output_poll();
        reverb_input(job);
        for (d = 0; d < 4; d++)
            job[d] += 2 * F;            // 80 samples of s32 at stride 2
    }
#else
    u16 n;
    for (n = 0; n < F; n++) {
        s16 half = quantize(((s64)quad[0][2 * n] + quad[1][2 * n] + quad[2][2 * n] + quad[3][2 * n]) << 15, 0x8000);
        mono[n] = quantize(2 * (s64)half * r->input_gain, 0x8000);
    }
#endif
    run_network(r, bus);
#ifdef __teak__
    job[0] = (u16)(unsigned)stereo;
    for (k = 0; k < 4; k++)
        job[1 + k] = (u16)(unsigned)quad[k];
    reverb_output(job);
#else
    for (n = 0; n < F; n++) {
        quad[0][2 * n] = stereo[2 * n];
        quad[1][2 * n] = stereo[2 * n + 1];
        quad[2][2 * n] = 0;
        quad[3][2 * n] = 0;
    }
#endif
}
