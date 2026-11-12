// SPDX-License-Identifier: BSD-2-Clause
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "output.h"
#include "../internal.h"
#include "../codec_events.h"
#include "../aac_backend.h"

struct opendspAudioStream {
    LightLock lock;
    audio_codec codec;
    audio_output output;
    opendspAudioInfo info;
    opendspAudioStats stats;
    opendspAudioLoopInfo loop, cache_loop;
    u8 *loop_pcm;
    u32 loop_frames, loop_capacity, loop_bytes;
    u64 decode_position;
    bool cache_active;
    opendspAudioState state;
    Result error;
    u32 generation, applied_generation, epoch, command_epoch, refs;
    u64 target, position;
    u32 backend, group;
    bool closing, play, paused, looping, eof, started, reset;
    bool rate_dirty, mix_dirty, control_dirty;
    float rate, volume, mix[12];
};

typedef struct {
    Thread thread;
    LightEvent event;
    unsigned group;
    bool frame_needed;
} audio_worker;
static audio_worker workers[2];
static u32 worker_channels[2];
static LightLock registry_lock, lifecycle_lock;
static opendspAudioStream *streams[24];
static u32 pool, epoch;
static bool ready, running, sleeping, shutdown_complete, events_ready;
static dspHookCookie hook_cookie;
static bool hooked;

static u32 current_epoch(void)
{
    return __atomic_load_n(&epoch, __ATOMIC_ACQUIRE);
}
static bool flag(const bool *p)
{
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}
static void set_flag(bool *p, bool value)
{
    __atomic_store_n(p, value, __ATOMIC_RELEASE);
}
static void cancel(opendspAudioStream *s)
{
    __atomic_add_fetch(&s->generation, 1, __ATOMIC_RELEASE);
}
static void wake_stream(opendspAudioStream *s)
{
    LightEvent_Signal(&workers[s->group].event);
}

void opendspAudioNotifyFrame(void)
{
    if (!flag(&events_ready) || !flag(&running))
        return;
    for (unsigned i = 0; i < 2; ++i)
        if (flag(&workers[i].frame_needed) && __atomic_load_n(&worker_channels[i], __ATOMIC_ACQUIRE))
            LightEvent_Signal(&workers[i].event);
}

void opendsp_audio_codec_notify(void)
{
    if (flag(&events_ready) && flag(&running) && __atomic_load_n(&worker_channels[1], __ATOMIC_ACQUIRE))
        LightEvent_Signal(&workers[1].event);
}

static void dsp_event(DSP_HookType type)
{
    if (type == DSPHOOK_ONSLEEP) {
        set_flag(&sleeping, true);
        // A stuck worker must not block sleep.
        u64 start = svcGetSystemTick();
        for (;;) {
            unsigned active = 0;
            LightLock_Lock(&registry_lock);
            for (unsigned i = 0; i < 24; ++i) if (streams[i]) active += streams[i]->refs;
            LightLock_Unlock(&registry_lock);
            if (!active || svcGetSystemTick() - start >= 3ull * SYSCLOCK_ARM11) break;
            svcSleepThread(500000);
        }
    }
    if (type == DSPHOOK_ONCANCEL) {
        set_flag(&sleeping, true);
        __atomic_add_fetch(&epoch, 1, __ATOMIC_RELEASE);
    }
}

void opendsp_audio_wakeup(bool restored)
{
    if (!restored) __atomic_add_fetch(&epoch, 1, __ATOMIC_RELEASE);
    LightLock_Lock(&registry_lock);
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i] && streams[i]->codec.direct_pcm) opendsp_audio_mp3_touch(&streams[i]->codec);
    LightLock_Unlock(&registry_lock);
    set_flag(&sleeping, false);
    if (flag(&events_ready) && flag(&running))
        for (unsigned i = 0; i < 2; ++i)
            if (__atomic_load_n(&worker_channels[i], __ATOMIC_ACQUIRE))
                LightEvent_Signal(&workers[i].event);
}

static u64 source_position(const opendspAudioStream *s)
{
    return s->output.source_position;
}

static void publish(opendspAudioStream *s, u64 elapsed)
{
    audio_output *o = &s->output;
    LightLock_Lock(&s->lock);
    s->info = s->codec.info;
    s->position = source_position(s);
    s->stats.codec_bytes = s->codec.memory + s->loop_bytes;
    s->stats.input_bytes = s->codec.stats.input_bytes;
    s->stats.decoded_frames = s->codec.stats.decoded_frames;
    s->stats.skipped_bytes = s->codec.stats.skipped_bytes;
    s->stats.read_ticks = s->codec.stats.read_ticks;
    s->stats.decode_ticks = s->codec.stats.decode_ticks;
    s->stats.queued_buffers = o->queued;
    u64 played = o->position - o->completed;
    u32 queued = played < o->queued_frames ? o->queued_frames - (u32)played : 0;
    s->stats.queued_frames = queued;
    if (queued > s->stats.peak_queued_frames)
        s->stats.peak_queued_frames = queued;
    if (s->started && !s->eof && !s->paused) {
        if (queued < s->stats.min_queued_frames)
            s->stats.min_queued_frames = queued;
        ++s->stats.queue_observations;
        s->stats.queue_frames_sum += queued;
    }
    s->stats.worker_ticks += elapsed;
    if (elapsed > s->stats.peak_worker_ticks)
        s->stats.peak_worker_ticks = elapsed;
    LightLock_Unlock(&s->lock);
}

static void fail(opendspAudioStream *s, Result r, u32 generation)
{
    LightLock_Lock(&s->lock);
    if (generation != __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE)) {
        LightLock_Unlock(&s->lock);
        return;
    }
    s->error = r;
    s->state = OPENDSP_AUDIO_ERROR;
    s->play = false;
    s->reset = false;
    s->applied_generation = generation;
    LightLock_Unlock(&s->lock);
    opendsp_audio_output_retire(&s->output, current_epoch());
}

// Cache loops during their first traversal.
#define AUDIO_LOOP_CACHE_BYTES (2u * 1024u * 1024u)
// Start once two buffers hold this.
#define START_FRAMES 8192u
static Result prime_loop(opendspAudioStream *s, u64 target)
{
    audio_codec *c = &s->codec;
    s->cache_active = false;
    LightLock_Lock(&s->lock);
    opendspAudioLoopInfo loop = s->loop;
    bool looping = s->looping;
    LightLock_Unlock(&s->lock);
    if (!loop.available && c->info.duration_known && c->info.total_frames) {
        loop.start_frame = 0;
        loop.end_frame = c->info.total_frames;
        loop.available = true;
    }
    if (!looping || !c->seekable)
        loop.available = false;
    u32 capacity = 0;
    if (loop.available) {
        u64 length = loop.end_frame - loop.start_frame;
        capacity = length <= AUDIO_LOOP_CACHE_BYTES / s->output.frame_bytes
                       ? (u32)length : c->info.sample_rate / 4;
        if (length > capacity && capacity > 16384)
            capacity = 16384;
    }
    if (capacity != s->loop_capacity || loop.start_frame != s->cache_loop.start_frame ||
        loop.end_frame != s->cache_loop.end_frame) {
        free(s->loop_pcm);
        s->loop_pcm = NULL;
        s->loop_frames = s->loop_capacity = s->loop_bytes = 0;
        if (capacity) {
            s->loop_pcm = malloc(capacity * s->output.frame_bytes);
            if (!s->loop_pcm)
                return OPENDSP_AUDIO_ERR_MEMORY;
            s->loop_capacity = capacity;
            s->loop_bytes = capacity * s->output.frame_bytes;
        }
    }
    s->cache_loop = loop;
    if (loop.available) {
        bool whole = capacity == loop.end_frame - loop.start_frame;
        if (target >= loop.end_frame)
            target = loop.start_frame;
        u32 needed = whole ? (target > loop.start_frame ? (u32)(target - loop.start_frame) : 0)
                           : capacity;
        if (s->loop_frames < needed) {
            Result r = c->ops->seek(c, loop.start_frame + s->loop_frames);
            if (R_FAILED(r))
                return r;
        }
        while (s->loop_frames < needed) {
            u32 got = 0;
            Result r = c->ops->read(c, s->loop_pcm + s->loop_frames * s->output.frame_bytes,
                                    needed - s->loop_frames, &got);
            if (R_FAILED(r))
                return r;
            if (!got)
                return OPENDSP_AUDIO_ERR_TRUNCATED;
            s->loop_frames += got;
        }
        s->cache_active = whole && s->loop_frames == capacity && target >= loop.start_frame;
    }
    s->decode_position = target;
    return s->cache_active ? 0 : c->ops->seek(c, target);
}

static void capture_loop(opendspAudioStream *s, const u8 *pcm, u64 source, u32 frames)
{
    if (!s->cache_loop.available || s->loop_frames == s->loop_capacity)
        return;
    u64 next = s->cache_loop.start_frame + s->loop_frames;
    if (source > next || source + frames <= next)
        return;
    u32 skip = (u32)(next - source);
    u32 count = frames - skip;
    if (count > s->loop_capacity - s->loop_frames)
        count = s->loop_capacity - s->loop_frames;
    memcpy(s->loop_pcm + s->loop_frames * s->output.frame_bytes,
           pcm + skip * s->output.frame_bytes, count * s->output.frame_bytes);
    s->loop_frames += count;
}

static void count_loops(opendspAudioStream *s, u32 count)
{
    LightLock_Lock(&s->lock);
    s->stats.loops += count;
    LightLock_Unlock(&s->lock);
}

static Result read_source(opendspAudioStream *s, int buffer, u32 *frames, u64 *source,
                           bool *repeated)
{
    audio_codec *c = &s->codec;
    audio_output *o = &s->output;
    opendspAudioLoopInfo *loop = &s->cache_loop;
    *frames = 0;
    *repeated = false;
    if (loop->available && s->decode_position >= loop->end_frame) {
        s->decode_position = loop->start_frame;
        s->cache_active = true;
        count_loops(s, 1);
    }
    *source = s->decode_position;
    if (s->cache_active) {
        u64 offset = s->decode_position - loop->start_frame;
        if (offset < s->loop_frames) {
            bool whole = s->loop_frames == loop->end_frame - loop->start_frame;
            u32 capacity = o->frames;
            u32 loops = 0;
            u8 *out = opendsp_audio_output_data(o, buffer);
            while (*frames < capacity && offset < s->loop_frames) {
                u32 n = s->loop_frames - (u32)offset;
                if (n > capacity - *frames)
                    n = capacity - *frames;
                memcpy(out + *frames * o->frame_bytes, s->loop_pcm + offset * o->frame_bytes,
                       n * o->frame_bytes);
                *frames += n;
                offset += n;
                s->decode_position += n;
                if (whole && offset == s->loop_frames) {
                    offset = 0;
                    s->decode_position = loop->start_frame;
                    ++loops;
                }
            }
            if (loops)
                count_loops(s, loops);
            *repeated = whole;
            return 0;
        }
        Result r = c->ops->seek(c, s->decode_position);
        if (R_FAILED(r))
            return r;
        s->cache_active = false;
    }
    u32 capacity = o->frames;
    if (loop->available && loop->end_frame - s->decode_position < capacity)
        capacity = (u32)(loop->end_frame - s->decode_position);
    u8 *out = opendsp_audio_output_data(o, buffer);
    // Combine short packets into output buffers.
    do {
        u32 got = 0;
        Result r = c->ops->read(c, out + *frames * o->frame_bytes, capacity - *frames, &got);
        if (R_FAILED(r))
            return r;
        capture_loop(s, out + *frames * o->frame_bytes, s->decode_position, got);
        *frames += got;
        s->decode_position += got;
        if (!got) {
            if (loop->available)
                return OPENDSP_AUDIO_ERR_TRUNCATED;
            break;
        }
    } while (*frames < capacity && c->seekable);
    return 0;
}

static bool step(opendspAudioStream *s)
{
    audio_output *o = &s->output;
    if (flag(&sleeping)) return false;
    u32 e = current_epoch();
    if (e != s->epoch) {
        s->epoch = e;
        LightLock_Lock(&s->lock);
        bool restart = s->command_epoch == e && s->reset;
        if (!restart) {
            s->error = OPENDSP_AUDIO_ERR_CONTEXT;
            s->state = OPENDSP_AUDIO_ERROR;
            s->play = false;
            s->reset = false;
            s->applied_generation = __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE);
        }
        LightLock_Unlock(&s->lock);
        opendsp_audio_output_retire(o, e);
    }
    if (flag(&sleeping))
        return false;
    LightLock_Lock(&s->lock);
    u32 generation = __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE);
    bool reset = s->reset || generation != s->applied_generation;
    bool play = s->play, paused = s->paused;
    if (play && s->eof && s->looping && s->codec.position)
        s->eof = false;
    bool control_changed = s->control_dirty;
    s->control_dirty = false;
    u64 target = s->target;
    if (s->mix_dirty) {
        float mix[12];
        for (unsigned i = 0; i < 12; ++i)
            mix[i] = s->mix[i] * s->volume;
        ndspChnSetMix(o->channel, mix);
        s->mix_dirty = false;
    }
    if (s->rate_dirty) {
        ndspChnSetRate(o->channel, s->rate);
        s->rate_dirty = false;
    }
    LightLock_Unlock(&s->lock);
    if (control_changed && s->codec.direct_pcm)
        opendsp_audio_mp3_touch(&s->codec);
    if (reset) {
        opendsp_audio_output_retire(o, e);
        if (!opendsp_audio_output_fenced(o, e, flag(&shutdown_complete)))
            return false;
        s->codec.in.expected = generation;
        Result r = prime_loop(s, target);
        if (generation != __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE))
            return false;
        if (R_FAILED(r)) {
            fail(s, r, generation);
            return false;
        }
        opendsp_audio_output_reset(o, s->decode_position);
        s->eof = s->started = false;
        LightLock_Lock(&s->lock);
        if (generation == __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE)) {
            s->reset = false;
            s->applied_generation = generation;
            s->error = 0;
            s->position = s->decode_position;
            opendsp_audio_output_configure(o, &s->info, s->rate);
            s->state = !s->play    ? OPENDSP_AUDIO_STOPPED
                       : s->paused ? OPENDSP_AUDIO_PAUSED
                                   : OPENDSP_AUDIO_BUFFERING;
        }
        LightLock_Unlock(&s->lock);
        return true;
    }
    if (!play || o->retiring)
        return false;
    opendsp_audio_output_reap(o);
    opendsp_audio_output_pause(o, paused || !s->started);
    if (paused)
        return false;
    if (s->started && !s->eof && !o->queued) {
        s->started = false;
        opendsp_audio_output_pause(o, true);
        LightLock_Lock(&s->lock);
        ++s->stats.underruns;
        ++s->stats.rebuffers;
        s->stats.min_queued_frames = 0;
        s->state = OPENDSP_AUDIO_BUFFERING;
        LightLock_Unlock(&s->lock);
    }
    bool progress = false;
    int buffer = opendsp_audio_output_free(o);
    if (s->codec.direct_pcm) {
        bool eof = false;
        s->codec.in.expected = generation;
        Result r = opendsp_audio_mp3_pump(&s->codec, o, &progress, &eof);
        if (generation != __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE))
            return true;
        if (r == OPENDSP_AUDIO_ERR_BUSY)
            return false;
        if (R_FAILED(r)) {
            fail(s, r, generation);
            return false;
        }
        if (eof) {
            if (s->looping && s->codec.position) {
                LightLock_Lock(&s->lock);
                ++s->stats.loops;
                LightLock_Unlock(&s->lock);
                r = s->codec.ops->seek(&s->codec, 0);
                o->next_source = 0;
                if (R_FAILED(r) && r != OPENDSP_AUDIO_ERR_BUSY)
                    fail(s, r, generation);
                return true;
            }
            s->eof = true;
        }
    } else if (!s->eof && buffer >= 0) {
        u32 frames = 0;
        u64 source;
        bool repeated;
        s->codec.in.expected = generation;
        Result r = read_source(s, buffer, &frames, &source, &repeated);
        LightLock_Lock(&s->lock);
        if (generation != __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE)) {
            LightLock_Unlock(&s->lock);
            return true;
        }
        if (r == OPENDSP_AUDIO_ERR_BUSY) {
            ++s->stats.busy_retries;
            LightLock_Unlock(&s->lock);
            return false;
        }
        if (R_FAILED(r)) {
            LightLock_Unlock(&s->lock);
            fail(s, r, generation);
            return false;
        }
        if (frames) {
            opendsp_audio_output_queue_source(o, buffer, frames, source,
                                              repeated ? s->cache_loop.start_frame : 0,
                                              repeated ? s->cache_loop.end_frame : 0);
            progress = true;
        } else if (s->looping && s->codec.position) {
            ++s->stats.loops;
            LightLock_Unlock(&s->lock);
            r = s->codec.ops->seek(&s->codec, 0);
            s->decode_position = 0;
            if (R_FAILED(r) && r != OPENDSP_AUDIO_ERR_BUSY)
                fail(s, r, generation);
            return true;
        } else
            s->eof = true;
        LightLock_Unlock(&s->lock);
    }
    LightLock_Lock(&s->lock);
    if (generation == __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE) && s->play && !s->paused) {
        if (s->eof && !o->queued) {
            s->state = OPENDSP_AUDIO_FINISHED;
            s->info = s->codec.info;
            s->position = source_position(s);
            s->play = false;
            s->started = false;
            opendsp_audio_output_pause(o, true);
        } else {
            if (!s->started && (o->queued == o->count || (o->queued >= 2 && o->queued_frames >= START_FRAMES) || (s->eof && o->queued)))
                s->started = true;
            if (s->started) {
                opendsp_audio_output_pause(o, false);
                s->state = s->eof ? OPENDSP_AUDIO_DRAINING : OPENDSP_AUDIO_PLAYING;
            }
        }
    }
    LightLock_Unlock(&s->lock);
    return s->codec.direct_pcm ? false : progress; /* pump already filled every free slot */
}

static void worker_main(void *arg)
{
    audio_worker *w = arg;
    unsigned first = 0;
    while (flag(&running)) {
        bool active = false, progress = false, frame_events = false;
        u32 candidates = __atomic_load_n(&worker_channels[w->group], __ATOMIC_ACQUIRE);
        for (unsigned n = 0; candidates && n < 24 && flag(&running); ++n) {
            unsigned i = (first + n) % 24;
            if (!(candidates & (1u << i)))
                continue;
            candidates &= ~(1u << i);
            LightLock_Lock(&registry_lock);
            opendspAudioStream *s = streams[i];
            if (s && !flag(&sleeping) && !s->closing && s->group == w->group)
                ++s->refs;
            else
                s = NULL;
            LightLock_Unlock(&registry_lock);
            if (!s)
                continue;
            LightLock_Lock(&s->lock);
            bool needs_work = !s->closing && (s->reset || (s->play && !s->paused) || s->control_dirty ||
                                              s->mix_dirty || s->rate_dirty || s->epoch != current_epoch());
            LightLock_Unlock(&s->lock);
            if (needs_work) {
                active = true;
                if (!s->codec.direct_pcm || s->output.retiring)
                    frame_events = true;
                u64 start = svcGetSystemTick();
                progress |= step(s);
                if (s->output.retiring)
                    frame_events = true;
                publish(s, svcGetSystemTick() - start);
            }
            LightLock_Lock(&registry_lock);
            --s->refs;
            LightLock_Unlock(&registry_lock);
        }
        first = (first + 1) % 24;
        set_flag(&w->frame_needed, frame_events);
        if (!flag(&running))
            break;
        if (!progress) {
            // Poll when codec IRQs are unavailable.
            if (active && (frame_events || !opendsp_codec_events_active()))
                LightEvent_WaitTimeout(&w->event, 5000000);
            else if (active)
                LightEvent_WaitTimeout(&w->event, 1000000000);
            else
                LightEvent_Wait(&w->event);
        }
    }
}

static void join_workers(void)
{
    set_flag(&running, false);
    if (flag(&events_ready))
        for (unsigned i = 0; i < 2; ++i)
            LightEvent_Signal(&workers[i].event);
    for (unsigned i = 0; i < 2; ++i)
        if (workers[i].thread) {
            threadJoin(workers[i].thread, U64_MAX);
            threadFree(workers[i].thread);
            workers[i].thread = NULL;
        }
}

Result opendspAudioInit(u32 channel_mask)
{
    if (!channel_mask || (channel_mask & ~0xffffffu))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&lifecycle_lock);
    if (flag(&ready)) {
        Result r = pool == channel_mask ? 0 : OPENDSP_AUDIO_ERR_BUSY;
        LightLock_Unlock(&lifecycle_lock);
        return r;
    }
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i]) {
            LightLock_Unlock(&lifecycle_lock);
            return OPENDSP_AUDIO_ERR_BUSY;
        }
    if (!opendsp_running) {
        LightLock_Unlock(&lifecycle_lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    if (!flag(&events_ready)) {
        for (unsigned i = 0; i < 2; ++i) {
            LightEvent_Init(&workers[i].event, RESET_ONESHOT);
            workers[i].group = i;
        }
        set_flag(&events_ready, true);
    }
    set_flag(&shutdown_complete, false);
    set_flag(&sleeping, false);
    set_flag(&running, true);
    s32 priority = 0x30;
    svcGetThreadPriority(&priority, CUR_THREAD_HANDLE);
    if (priority > 0x18)
        --priority;
    for (unsigned i = 0; i < 2; ++i) {
        set_flag(&workers[i].frame_needed, false);
        workers[i].thread = threadCreate(worker_main, &workers[i], 16 * 1024, priority, -2, false);
        if (!workers[i].thread) {
            join_workers();
            LightLock_Unlock(&lifecycle_lock);
            return OPENDSP_AUDIO_ERR_MEMORY;
        }
    }
    dspHook(&hook_cookie, dsp_event);
    hooked = true;
    pool = channel_mask;
    set_flag(&ready, true);
    LightLock_Unlock(&lifecycle_lock);
    return 0;
}

Result opendspAudioExit(void)
{
    LightLock_Lock(&lifecycle_lock);
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i]) {
            LightLock_Unlock(&lifecycle_lock);
            return OPENDSP_AUDIO_ERR_BUSY;
        }
    set_flag(&ready, false);
    join_workers();
#ifdef OPENDSP_CODEC_BANKS
    aac_backend_release_idle();
#endif
    if (hooked) {
        dspUnhook(&hook_cookie);
        hooked = false;
    }
    LightLock_Unlock(&lifecycle_lock);
    return 0;
}

Result opendspAudioOpen(opendspAudioStream **out, const char *path, const opendspAudioConfig *config)
{
    if (!out)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    *out = NULL;
    opendspAudioConfig cfg = OPENDSP_AUDIO_CONFIG_INIT;
    if (config) {
        if (config->struct_size < sizeof(cfg))
            return OPENDSP_AUDIO_ERR_ARGUMENT;
        cfg = *config;
    }
    if (!path || !*path || cfg.channel < -1 || cfg.channel >= 24 ||
        (cfg.buffer_count && (cfg.buffer_count < 2 || cfg.buffer_count > 8)) ||
        (cfg.buffer_frames && (cfg.buffer_frames < 256 || cfg.buffer_frames > 16384)) ||
        cfg.aac_backend > OPENDSP_AAC_BACKEND_DSP ||
        (cfg.flags & ~(OPENDSP_AUDIO_LOOP | OPENDSP_AUDIO_PROFILE | OPENDSP_AUDIO_MSU1_PCM)))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&lifecycle_lock);
    if (!flag(&ready) || flag(&sleeping)) {
        LightLock_Unlock(&lifecycle_lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    int channel = cfg.channel;
    if (channel < 0) {
        for (channel = 0; channel < 24; ++channel)
            if ((pool & (1u << channel)) && !streams[channel])
                break;
    }
    if (channel >= 24 || !(pool & (1u << channel)) || streams[channel]) {
        LightLock_Unlock(&lifecycle_lock);
        return OPENDSP_AUDIO_ERR_BUSY;
    }
    opendspAudioStream *s = calloc(1, sizeof(*s));
    Result r = OPENDSP_AUDIO_ERR_MEMORY;
    if (!s)
        goto done;
    LightLock_Init(&s->lock);
    s->epoch = current_epoch();
    s->command_epoch = s->epoch;
    r = opendsp_audio_codec_open(&s->codec, path, cfg.aac_backend, cfg.buffer_count, cfg.buffer_frames, cfg.flags);
    if (R_FAILED(r))
        goto fail;
    if (s->epoch != current_epoch() || flag(&sleeping)) {
        r = OPENDSP_AUDIO_ERR_CONTEXT;
        goto fail;
    }
    s->info = s->codec.info;
    s->backend = s->codec.backend;
    s->loop = s->codec.loop;
    if ((cfg.flags & OPENDSP_AUDIO_LOOP) && s->loop.status == OPENDSP_AUDIO_LOOP_INVALID) {
        r = OPENDSP_AUDIO_ERR_FORMAT;
        goto fail;
    }
    s->group = s->info.format != OPENDSP_AUDIO_WAV && s->info.format != OPENDSP_AUDIO_MSU1;
    u32 count = cfg.buffer_count     ? cfg.buffer_count
                : s->codec.direct_pcm ? AUDIO_MP3_DSP_BUFFERS
                                      : 4;
    u32 frames = cfg.buffer_frames                       ? cfg.buffer_frames
                 : s->info.format == OPENDSP_AUDIO_VORBIS ? 4096
                 : s->info.format == OPENDSP_AUDIO_OPUS   ? 4096
                 : s->info.format == OPENDSP_AUDIO_OPUZ   ? 4096
                 : s->info.format == OPENDSP_AUDIO_MP3   ? 1152
                 : s->group                              ? 1024
                                                         : 2048;
    r = opendsp_audio_output_open(&s->output, channel, count, frames, &s->info, s->codec.direct_pcm,
                                  s->codec.direct_stride);
    if (R_FAILED(r))
        goto fail;
    s->rate = s->info.sample_rate;
    s->volume = 1;
    s->looping = (cfg.flags & OPENDSP_AUDIO_LOOP) != 0;
    s->stats.struct_size = sizeof(s->stats);
    s->stats.capacity_frames = count * frames;
    s->stats.min_queued_frames = count * frames;
    s->stats.linear_bytes = s->output.bytes;
    s->stats.codec_bytes = s->codec.memory;
    s->codec.in.generation = &s->generation;
    ndspChnGetMix(channel, s->mix);
    LightLock_Lock(&registry_lock);
    streams[channel] = s;
    __atomic_fetch_or(&worker_channels[s->group], 1u << channel, __ATOMIC_RELEASE);
    LightLock_Unlock(&registry_lock);
    *out = s;
    wake_stream(s);
    goto done;
fail:
    opendsp_audio_codec_close(&s->codec);
    free(s);
done:
    LightLock_Unlock(&lifecycle_lock);
    return r;
}

typedef enum { AUDIO_PLAY, AUDIO_PAUSE, AUDIO_RESUME, AUDIO_STOP, AUDIO_SEEK } audio_command;

static Result command(opendspAudioStream *s, audio_command cmd, u64 frame)
{
    if (!s)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready) || flag(&sleeping)) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    if (cmd == AUDIO_SEEK && ((!s->codec.seekable && frame) ||
                              (s->info.duration_known && frame > s->info.total_frames))) {
        LightLock_Unlock(&s->lock);
        return !s->codec.seekable && frame ? OPENDSP_AUDIO_ERR_UNSUPPORTED
                                            : OPENDSP_AUDIO_ERR_ARGUMENT;
    }
    if (cmd == AUDIO_PLAY) {
        if (!s->play) {
            if (s->state == OPENDSP_AUDIO_FINISHED || s->state == OPENDSP_AUDIO_ERROR)
                s->target = 0;
            s->play = true;
            s->paused = false;
            s->reset = true;
            cancel(s);
            s->command_epoch = current_epoch();
            s->state = OPENDSP_AUDIO_BUFFERING;
        }
    } else if (cmd == AUDIO_PAUSE) {
        if (s->play) {
            s->paused = true;
            s->control_dirty = true;
            s->state = OPENDSP_AUDIO_PAUSED;
        }
    } else if (cmd == AUDIO_RESUME) {
        if (s->play) {
            s->paused = false;
            s->control_dirty = true;
            s->state = OPENDSP_AUDIO_BUFFERING;
        }
    } else {
        s->target = cmd == AUDIO_STOP ? 0 : frame;
        if (cmd == AUDIO_STOP) {
            s->play = false;
            s->paused = false;
        }
        s->reset = true;
        cancel(s);
        s->command_epoch = current_epoch();
        s->state = OPENDSP_AUDIO_STOPPING;
    }
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return 0;
}
Result opendspAudioPlay(opendspAudioStream *s)
{
    return command(s, AUDIO_PLAY, 0);
}
Result opendspAudioPause(opendspAudioStream *s)
{
    return command(s, AUDIO_PAUSE, 0);
}
Result opendspAudioResume(opendspAudioStream *s)
{
    return command(s, AUDIO_RESUME, 0);
}
Result opendspAudioStop(opendspAudioStream *s)
{
    return command(s, AUDIO_STOP, 0);
}
Result opendspAudioSeek(opendspAudioStream *s, u64 frame)
{
    return command(s, AUDIO_SEEK, frame);
}

static Result close_within(opendspAudioStream **handle, u64 timeout)
{
    if (!handle)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    if (!*handle)
        return 0;
    opendspAudioStream *s = *handle;
    if (LightLock_TryLock(&lifecycle_lock))
        return OPENDSP_AUDIO_ERR_BUSY;
    LightLock_Lock(&registry_lock);
    LightLock_Lock(&s->lock);
    if (!s->closing) {
        s->closing = true;
        s->play = false;
        s->state = OPENDSP_AUDIO_STOPPING;
        cancel(s);
    }
    __atomic_fetch_and(&worker_channels[s->group], ~(1u << s->output.channel), __ATOMIC_RELEASE);
    LightLock_Unlock(&s->lock);
    LightLock_Unlock(&registry_lock);
    wake_stream(s);
    u64 start = svcGetSystemTick();
    for (;;) {
        LightLock_Lock(&registry_lock);
        bool busy = s->refs != 0;
        LightLock_Unlock(&registry_lock);
        if (!busy) {
            opendsp_audio_output_retire(&s->output, current_epoch());
            if (opendsp_audio_output_fenced(&s->output, current_epoch(), flag(&shutdown_complete)))
                break;
        }
        if (svcGetSystemTick() - start >= timeout) {
            LightLock_Unlock(&lifecycle_lock);
            return OPENDSP_AUDIO_ERR_BUSY;
        }
        svcSleepThread(1000000);
    }
    opendsp_audio_codec_close(&s->codec);
    opendsp_audio_output_close(&s->output);
    free(s->loop_pcm);
    // The next stream inherits this mix, unscaled.
    ndspChnSetMix(s->output.channel, s->mix);
    LightLock_Lock(&registry_lock);
    streams[s->output.channel] = NULL;
    LightLock_Unlock(&registry_lock);
    free(s);
    *handle = NULL;
    LightLock_Unlock(&lifecycle_lock);
    return 0;
}

Result opendspAudioClose(opendspAudioStream **handle)
{
    return close_within(handle, SYSCLOCK_ARM11);
}

Result opendsp_audio_close_poll(opendspAudioStream **handle)
{
    return close_within(handle, 0);
}

Result opendspAudioSetLoop(opendspAudioStream *s, bool loop)
{
    if (!s)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready) || flag(&sleeping)) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    if (loop && s->loop.status == OPENDSP_AUDIO_LOOP_INVALID) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_FORMAT;
    }
    if (s->looping != loop) {
        s->looping = loop;
        if (s->codec.seekable) {
            if (s->play && !s->reset)
                s->target = s->position;
            s->reset = true;
            cancel(s);
            s->command_epoch = current_epoch();
        }
    }
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return 0;
}

static Result loop_range(opendspAudioStream *s, bool clear, u64 start, u64 end)
{
    if (!s || (!clear && start >= end))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    Result r = 0;
    if (s->closing || !flag(&ready) || flag(&sleeping))
        r = OPENDSP_AUDIO_ERR_STATE;
    else if (!s->codec.seekable)
        r = OPENDSP_AUDIO_ERR_UNSUPPORTED;
    else if (clear && s->looping && s->codec.loop.status == OPENDSP_AUDIO_LOOP_INVALID)
        r = OPENDSP_AUDIO_ERR_FORMAT;
    else if (!clear && (!s->info.duration_known || end > s->info.total_frames))
        r = OPENDSP_AUDIO_ERR_ARGUMENT;
    else {
        if (clear)
            s->loop = s->codec.loop;
        else {
            memset(&s->loop, 0, sizeof(s->loop));
            s->loop.struct_size = sizeof(s->loop);
            s->loop.available = true;
            s->loop.status = OPENDSP_AUDIO_LOOP_VALID;
            s->loop.start_frame = start;
            s->loop.end_frame = end;
        }
        if (s->play && !s->reset)
            s->target = s->position;
        s->reset = true;
        cancel(s);
        s->command_epoch = current_epoch();
    }
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return r;
}

Result opendspAudioSetLoopRange(opendspAudioStream *s, u64 start, u64 end)
{
    return loop_range(s, false, start, end);
}

Result opendspAudioClearLoopRange(opendspAudioStream *s)
{
    return loop_range(s, true, 0, 0);
}

Result opendspAudioGetLoopInfo(opendspAudioStream *s, opendspAudioLoopInfo *info)
{
    if (!s || !info || info->struct_size < sizeof(*info))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    *info = s->loop;
    LightLock_Unlock(&s->lock);
    return 0;
}
Result opendspAudioSetVolume(opendspAudioStream *s, float volume)
{
    if (!s || !isfinite(volume) || volume < 0 || volume > 1)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    s->volume = volume;
    s->mix_dirty = true;
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return 0;
}
Result opendspAudioSetMix(opendspAudioStream *s, const float mix[12])
{
    if (!s || !mix)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    for (unsigned i = 0; i < 12; ++i)
        if (!isfinite(mix[i]) || mix[i] < -1 || mix[i] > 1)
            return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    memcpy(s->mix, mix, sizeof(s->mix));
    s->mix_dirty = true;
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return 0;
}
Result opendspAudioSetRate(opendspAudioStream *s, float rate)
{
    if (!s || !isfinite(rate) || rate < 1 || rate > 4 * 32728)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) {
        LightLock_Unlock(&s->lock);
        return OPENDSP_AUDIO_ERR_STATE;
    }
    s->rate = rate;
    s->rate_dirty = true;
    LightLock_Unlock(&s->lock);
    wake_stream(s);
    return 0;
}

Result opendspAudioGetInfo(opendspAudioStream *s, opendspAudioInfo *info)
{
    if (!s || !info || info->struct_size < sizeof(*info))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    *info = s->info;
    LightLock_Unlock(&s->lock);
    return 0;
}
Result opendspAudioGetStats(opendspAudioStream *s, opendspAudioStats *stats)
{
    if (!s || !stats || stats->struct_size < sizeof(*stats))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    *stats = s->stats;
    LightLock_Unlock(&s->lock);
    return 0;
}
Result opendspAudioGetDecoderStats(opendspAudioStream *s, opendspAudioDecoderStats *stats)
{
    if (!s || !stats || stats->struct_size < sizeof(*stats))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    Result r = OPENDSP_AUDIO_ERR_UNSUPPORTED;
    if (!s->closing && flag(&ready) && s->codec.ops && s->codec.ops->stats)
        r = s->codec.ops->stats(&s->codec, stats);
    LightLock_Unlock(&s->lock);
    return r;
}
opendspAudioState opendspAudioGetState(opendspAudioStream *s)
{
    if (!s)
        return OPENDSP_AUDIO_ERROR;
    LightLock_Lock(&s->lock);
    opendspAudioState value = s->state;
    LightLock_Unlock(&s->lock);
    return value;
}
u64 opendspAudioGetPosition(opendspAudioStream *s)
{
    if (!s)
        return 0;
    LightLock_Lock(&s->lock);
    u64 value = s->position;
    LightLock_Unlock(&s->lock);
    return value;
}
u32 opendspAudioGetBackend(opendspAudioStream *s)
{
    return s ? s->backend : OPENDSP_AAC_BACKEND_AUTO;
}
s32 opendspAudioGetChannel(opendspAudioStream *s)
{
    return s ? s->output.channel : -1;
}
Result opendspAudioGetLastError(opendspAudioStream *s)
{
    if (!s)
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    Result value = s->error;
    LightLock_Unlock(&s->lock);
    return value;
}

void opendsp_audio_before_exit(void)
{
    LightLock_Lock(&lifecycle_lock);
    set_flag(&ready, false);
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i])
            cancel(streams[i]);
    join_workers();
    if (hooked) {
        dspUnhook(&hook_cookie);
        hooked = false;
    }
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i]) {
            opendsp_audio_output_retire(&streams[i]->output, current_epoch());
            if (streams[i]->output.borrowed)
                opendsp_audio_mp3_stop(&streams[i]->codec);
            else
                opendsp_audio_codec_close(&streams[i]->codec);
        }
}

void opendsp_audio_after_exit(bool unloaded)
{
    set_flag(&shutdown_complete, unloaded);
    for (unsigned i = 0; i < 24; ++i)
        if (streams[i]) {
            if (unloaded && streams[i]->output.borrowed)
                opendsp_audio_codec_close(&streams[i]->codec);
            LightLock_Lock(&streams[i]->lock);
            streams[i]->state = OPENDSP_AUDIO_ERROR;
            streams[i]->error = OPENDSP_AUDIO_ERR_CONTEXT;
            streams[i]->play = false;
            LightLock_Unlock(&streams[i]->lock);
        }
    LightLock_Unlock(&lifecycle_lock);
}
