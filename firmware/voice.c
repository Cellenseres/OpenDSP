// SPDX-License-Identifier: BSD-2-Clause
// Voice engine: queue, PCM/ADPCM fetch, resampling, stream end.
#include "voice.h"
#include "dma.h"
#include "polyphase_table.h"

static s16 window_buf[4 + 2 * WINDOW_FRAMES];
static s16 *const window = window_buf + 4; // 4 history slots before

// teak-llvm lacks i32 MUL; opaque shifts stop folding
static volatile u16 shift5 = 5, shift7 = 7;
static u32 times160(u32 x)
{
    return (x << shift7) + (x << shift5);
}

void voice_reset(voice_t *v)
{
    volatile u16 *p = (volatile u16 *)v;
    u16 i, slot = v->coef_slot;
    for (i = 0; i < sizeof(*v) / sizeof(u16); i++)
        p[i] = 0;
    v->coef_slot = slot;
    v->slot_version = 0xFFFF;
    v->step = 0x10000;
    v->interp = INTERP_LINEAR;
}

int voice_queue(voice_t *v, const wavebuf_t *buf)
{
    if (v->queued >= QUEUE_SIZE)
        return -1;
    v->queue[v->queued] = *buf;
    v->queue[v->queued].played = 0;
    v->queued++;
    return 0;
}

// DSP-ADPCM: 8-byte frames, header plus 14 nibbles
#define ADPCM_CHUNK 32
#define COEF_SLOTS 32

typedef struct {
    const s16 *coefs;
    const u16 *nibbles;
    s16 *dst;
    u16 count, scale;
} adpcm_job_t;
typedef struct {
    const u16 *raw;
    const s16 *coefs;
    s16 *dst;
    u16 frames;
    u16 *nibbles;
    u16 pad;
    adpcm_job_t job;
} adpcm_block_job_t;

void adpcm_run(const adpcm_job_t *job);
void adpcm_unpack(const u16 *frame, u16 *nibbles);
void adpcm_block(adpcm_block_job_t *job);

static adpcm_job_t adpcm_job;
static adpcm_block_job_t block_job;
static s16 coef_slots[COEF_SLOTS][16] YMEM;
static u16 nibbles[14];
static s16 discard_buf[2 + 64];
static s16 *const discard = discard_buf + 2;

// dst[-1], dst[-2] hold the decoder history
static void adpcm_decode(voice_t *v, u16 n, s16 *dst)
{
    u32 frame = v->adpcm_frame;
    u16 nib = v->adpcm_nibble, total = n, k;
    s16 *coefs = coef_slots[v->coef_slot & (COEF_SLOTS - 1)];
    s16 saved1 = dst[-1], saved2 = dst[-2];
    s16 *d = dst;
    if (v->slot_version != v->coef_version) {
        for (k = 0; k < 16; k++)
            coefs[k] = v->coefs[k];
        v->slot_version = v->coef_version;
    }
    dst[-1] = v->yn1;
    dst[-2] = v->yn2;
    while (n) {
        u16 frames = 0, f, rest = (u16)(n + nib);
        const u16 *raw;
        while (rest && frames < ADPCM_CHUNK) {
            frames++;
            rest = rest > 14 ? (u16)(rest - 14) : 0;
        }
        raw = dma_fetch_words(v->current.address + (frame << 3), (u16)(frames << 2));
        for (f = 0; f < frames && n; f++) {
            if (nib == 0 && n >= 14) {
                u16 whole = 0, left = n;
                while (f + whole < frames && left >= 14) {
                    whole++;
                    left = (u16)(left - 14);
                }
                block_job.raw = &raw[f << 2];
                block_job.coefs = coefs;
                block_job.dst = d;
                block_job.frames = whole;
                block_job.nibbles = nibbles;
                block_job.job.nibbles = nibbles;
                block_job.job.count = 14;
                adpcm_block(&block_job);
                d += n - left;
                n = left;
                frame += whole;
                f += whole;
                if (!n || f >= frames)
                    break;
            }
            {
                const u16 *w = &raw[f << 2];
                u16 header = w[0] & 0xFF, pair = ((header >> 4) & 7) << 1, m;
                adpcm_unpack(w, nibbles);
                m = (u16)(14 - nib);
                if (m > n)
                    m = n;
                adpcm_job.coefs = &coefs[pair];
                adpcm_job.nibbles = &nibbles[nib];
                adpcm_job.dst = d;
                adpcm_job.count = m;
                adpcm_job.scale = (u16)((header & 0xF) - 1);
                adpcm_run(&adpcm_job);
                d += m;
                n -= m;
                nib += m;
                if (nib == 14) {
                    nib = 0;
                    frame++;
                }
            }
        }
    }
    if (total >= 2) {
        v->yn1 = dst[total - 1];
        v->yn2 = dst[total - 2];
    } else if (total == 1) {
        v->yn2 = v->yn1;
        v->yn1 = dst[0];
    }
    dst[-1] = saved1;
    dst[-2] = saved2;
    v->adpcm_frame = frame;
    v->adpcm_nibble = nib;
}

// lowest buffer id plays next; looping buffers requeue
static int dequeue(voice_t *v)
{
    u16 i, best = 0;
    if (!v->queued)
        return 0;
    for (i = 1; i < v->queued; i++)
        if (v->queue[i].id < v->queue[best].id)
            best = i;
    v->current = v->queue[best];
    for (i = best; i + 1 < v->queued; i++)
        v->queue[i] = v->queue[i + 1];
    v->queued--;
    v->position = v->current.played ? 0 : v->current.start;
    if (!v->current.played) {
        v->last_id = v->current_id;
        v->id_changed = 1;
    }
    if (v->current.looping) {
        v->queue[v->queued] = v->current;
        v->queue[v->queued].played = 1;
        v->queued++;
    }
    v->has_current = 1;
    if (v->format == FMT_ADPCM) {
        u32 p = 0;
        v->adpcm_frame = 0;
        v->adpcm_nibble = 0;
        if (v->current.adpcm_context) {
            v->yn1 = v->current.yn1;
            v->yn2 = v->current.yn2;
        }
        while (p < v->position) { // skip to the start offset
            u16 m = (v->position - p > 64) ? 64 : (u16)(v->position - p);
            adpcm_decode(v, m, discard);
            p += m;
        }
    }
    return 1;
}

// window slots reported in the status
static u16 track_index[3], track_id[3];
static u32 track_position[3];
static fetch_job_t fetch_job;
static resample_job_t resample_job;

// fill window[at, at + n); returns real samples
static u16 pull(voice_t *v, u16 at, u16 n)
{
    u16 got = 0, shift = v->stereo ? 1 : 0, p;
    while (got < n) {
        u32 left;
        u16 m;
        if (!v->has_current || v->position >= v->current.length) {
            v->has_current = 0;
            if (!dequeue(v))
                break;
            continue;
        }
        left = v->current.length - v->position;
        m = (left < (u32)(n - got)) ? (u16)left : (u16)(n - got);
        if (v->format == FMT_ADPCM) {
            adpcm_decode(v, m, &window[at + got]);
        } else {
            u32 offset = v->position;
            if (v->format == FMT_PCM16)
                offset <<= 1;
            if (v->stereo)
                offset <<= 1;
            fetch_job.address = v->current.address + offset;
            fetch_job.count = m;
            fetch_job.format = v->format;
            fetch_job.stereo = v->stereo;
            fetch_job.dst = &window[(at + got) << shift];
            fetch_samples(&fetch_job);
        }
        for (p = 0; p < 3; p++) {
            u16 ix = track_index[p];
            if (ix >= at + got && ix < at + got + m) {
                track_id[p] = v->current.id;
                track_position[p] = v->position + (ix - at - got);
            }
        }
        v->last_valid_id = v->current.id;
        v->position += m;
        got += m;
    }
    for (p = got; p < n; p++) { // past the end: silence
        window[(at + p) << shift] = 0;
        if (shift)
            window[((at + p) << 1) + 1] = 0;
    }
    return got;
}

int voice_render(voice_t *v, s16 *out)
{
    u16 k, shift, carries, last_input, available, head;
    u32 t;
    if (!v->enabled) {
        for (k = 0; k < 2 * FRAME_SAMPLES; k++)
            out[k] = 0;
        return 0;
    }
    if (v->step > MAX_STEP)
        v->step = MAX_STEP;
    shift = v->stereo ? 1 : 0;
    t = (u32)v->frac + times160(v->step) - v->step;
    last_input = (u16)(t >> 16);
    carries = (u16)((t + v->step) >> 16);

    if (!v->primed) {
        v->primed = 1;
        track_index[0] = 0;
        track_index[1] = 1;
        track_index[2] = 0xFFFF;
        track_id[0] = track_id[1] = 0;
        track_position[0] = track_position[1] = 0;
        v->head_valid = pull(v, 0, 2);
        v->x0[0] = window[0];
        v->x0[1] = window[shift ? 1 : 0];
        v->x1[0] = window[1 << shift];
        v->x1[1] = window[shift ? 3 : 1];
        v->x0_id = track_id[0];
        v->x0_position = track_position[0];
        v->x1_id = track_id[1];
        v->x1_position = track_position[1];
    }
    if (shift) {
        window[0] = v->x0[0];
        window[1] = v->x0[1];
        window[2] = v->x1[0];
        window[3] = v->x1[1];
        window[-2] = v->h1[0];
        window[-1] = v->h1[1];
        window[-4] = v->h2[0];
        window[-3] = v->h2[1];
    } else {
        window[0] = v->x0[0];
        window[1] = v->x1[0];
        window[-1] = v->h1[0];
        window[-2] = v->h2[0];
    }
    track_index[0] = last_input;
    track_index[1] = carries;
    track_index[2] = carries + 1;
    for (k = 0; k < 3; k++) {
        u16 ix = track_index[k];
        if (ix == 0) {
            track_id[k] = v->x0_id;
            track_position[k] = v->x0_position;
        } else if (ix == 1) {
            track_id[k] = v->x1_id;
            track_position[k] = v->x1_position;
        }
    }
    head = v->head_valid;
    if (head == 2) {
        available = 2 + pull(v, 2, carries);
    } else {
        available = head;
        for (k = 2; k < carries + 2; k++) {
            window[k << shift] = 0;
            if (shift)
                window[(k << 1) + 1] = 0;
        }
    }

    resample_job.window = window;
    resample_job.out = out;
    resample_job.step = v->step;
    resample_job.stereo = v->stereo;
    resample_job.interp = v->interp;
    resample_job.frac = v->frac;
    resample_job.count = FRAME_SAMPLES;
    resample_job.table = polyphase_table;
    resample(&resample_job);

    if (last_input >= available) {
        // stream ended: silence from the first missing input
        t = v->frac;
        for (k = 0; k < FRAME_SAMPLES; k++) {
            if ((u16)(t >> 16) >= available) {
                out[2 * k] = 0;
                out[2 * k + 1] = 0;
            }
            t += v->step;
        }
        v->enabled = 0;
        v->last_id = v->last_valid_id;
        v->current_id = 0;
        v->id_changed = 1;
        return 0;
    }
    v->current_id = track_id[0];
    v->status_position = track_position[0];
    if (shift) {
        v->h1[0] = window[(carries << 1) - 2];
        v->h1[1] = window[(carries << 1) - 1];
        v->h2[0] = window[(carries << 1) - 4];
        v->h2[1] = window[(carries << 1) - 3];
    } else {
        v->h1[0] = window[carries - 1];
        v->h2[0] = window[carries - 2];
    }
    v->x0[0] = window[carries << shift];
    v->x0[1] = window[shift ? (carries << 1) + 1 : carries];
    v->x1[0] = window[(carries + 1) << shift];
    v->x1[1] = window[shift ? ((carries + 1) << 1) + 1 : carries + 1];
    v->x0_id = track_id[1];
    v->x0_position = track_position[1];
    v->x1_id = track_id[2];
    v->x1_position = track_position[2];
    v->head_valid = (carries < available) ? ((carries + 1 < available) ? 2 : 1) : 0;
    v->frac = (u16)(v->frac + (u16)times160(v->step));
    return 1;
}
