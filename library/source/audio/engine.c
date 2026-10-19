// SPDX-License-Identifier: BSD-2-Clause
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "output.h"
#include "../internal.h"

struct opendspAudioStream {
    LightLock lock;
    audio_codec codec;
    audio_output output;
    opendspAudioInfo info;
    opendspAudioStats stats;
    opendspAudioState state;
    Result error;
    u32 generation, applied_generation, epoch, command_epoch, refs;
    u64 target, position;
    u32 backend, group;
    bool closing, play, paused, looping, looped, eof, started, reset;
    bool rate_dirty, mix_dirty, control_dirty;
    float rate, volume, mix[12];
};

typedef struct { Thread thread; LightEvent event; unsigned group; } audio_worker;
static audio_worker workers[2];
static LightLock registry_lock, lifecycle_lock;
static opendspAudioStream *streams[24];
static u32 pool, epoch;
static bool ready, running, sleeping, shutdown_complete, events_ready;
static dspHookCookie hook_cookie;
static bool hooked;

static u32 current_epoch(void) { return __atomic_load_n(&epoch, __ATOMIC_ACQUIRE); }
static bool flag(const bool *p) { return __atomic_load_n(p, __ATOMIC_ACQUIRE); }
static void set_flag(bool *p, bool value) { __atomic_store_n(p, value, __ATOMIC_RELEASE); }
static void cancel(opendspAudioStream *s) { __atomic_add_fetch(&s->generation, 1, __ATOMIC_RELEASE); }

void opendspAudioNotifyFrame(void)
{
    if (!flag(&events_ready)) return;
    LightEvent_Signal(&workers[0].event);
    LightEvent_Signal(&workers[1].event);
}

static void dsp_event(DSP_HookType type)
{
    if (type != DSPHOOK_ONWAKEUP) set_flag(&sleeping, true);
    __atomic_add_fetch(&epoch, 1, __ATOMIC_RELEASE);
    if (type == DSPHOOK_ONWAKEUP) set_flag(&sleeping, false);
    opendspAudioNotifyFrame();
}

static u64 source_position(const opendspAudioStream *s)
{
    u64 position = s->output.position;
    if ((s->looping || s->looped) && s->info.duration_known && s->info.total_frames) {
        position %= s->info.total_frames;
        if (s->state == OPENDSP_AUDIO_FINISHED && !position && s->output.position)
            position = s->info.total_frames;
    }
    return position;
}

static void publish(opendspAudioStream *s, u64 elapsed)
{
    audio_output *o = &s->output;
    LightLock_Lock(&s->lock);
    s->info = s->codec.info;
    s->position = source_position(s);
    s->stats.input_bytes = s->codec.stats.input_bytes;
    s->stats.decoded_frames = s->codec.stats.decoded_frames;
    s->stats.skipped_bytes = s->codec.stats.skipped_bytes;
    s->stats.read_ticks = s->codec.stats.read_ticks;
    s->stats.decode_ticks = s->codec.stats.decode_ticks;
    s->stats.queued_buffers = o->queued;
    u64 played = o->position - o->completed;
    u32 queued = played < o->queued_frames ? o->queued_frames - (u32)played : 0;
    s->stats.queued_frames = queued;
    if (queued > s->stats.peak_queued_frames) s->stats.peak_queued_frames = queued;
    if (s->started && !s->eof && !s->paused) {
        if (queued < s->stats.min_queued_frames) s->stats.min_queued_frames = queued;
        ++s->stats.queue_observations;
        s->stats.queue_frames_sum += queued;
    }
    s->stats.worker_ticks += elapsed;
    if (elapsed > s->stats.peak_worker_ticks) s->stats.peak_worker_ticks = elapsed;
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

static bool step(opendspAudioStream *s)
{
    audio_output *o = &s->output;
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
    if (flag(&sleeping)) return false;
    LightLock_Lock(&s->lock);
    u32 generation = __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE);
    bool reset = s->reset || generation != s->applied_generation;
    bool play = s->play, paused = s->paused;
    if (play && s->eof && s->looping && s->codec.position) s->eof = false;
    s->control_dirty = false;
    u64 target = s->target;
    if (s->mix_dirty) {
        float mix[12];
        for (unsigned i = 0; i < 12; ++i) mix[i] = s->mix[i] * s->volume;
        ndspChnSetMix(o->channel, mix);
        s->mix_dirty = false;
    }
    if (s->rate_dirty) { ndspChnSetRate(o->channel, s->rate); s->rate_dirty = false; }
    LightLock_Unlock(&s->lock);
    if (reset) {
        opendsp_audio_output_retire(o, e);
        if (!opendsp_audio_output_fenced(o, e, flag(&shutdown_complete))) return false;
        s->codec.in.expected = generation;
        Result r = s->codec.ops->seek(&s->codec, target);
        if (generation != __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE)) return false;
        if (R_FAILED(r)) { fail(s, r, generation); return false; }
        opendsp_audio_output_reset(o, target);
        s->eof = s->started = s->looped = false;
        LightLock_Lock(&s->lock);
        if (generation == __atomic_load_n(&s->generation, __ATOMIC_ACQUIRE)) {
            s->reset = false;
            s->applied_generation = generation;
            s->error = 0;
            s->position = target;
            opendsp_audio_output_configure(o, &s->info, s->rate);
            s->state = !s->play ? OPENDSP_AUDIO_STOPPED : s->paused ? OPENDSP_AUDIO_PAUSED : OPENDSP_AUDIO_BUFFERING;
        }
        LightLock_Unlock(&s->lock);
        return true;
    }
    if (!play || o->retiring) return false;
    opendsp_audio_output_reap(o);
    ndspChnSetPaused(o->channel, paused || !s->started);
    if (paused) return false;
    if (s->started && !s->eof && !o->queued) {
        s->started = false;
        ndspChnSetPaused(o->channel, true);
        LightLock_Lock(&s->lock);
        ++s->stats.underruns;
        ++s->stats.rebuffers;
        s->stats.min_queued_frames = 0;
        s->state = OPENDSP_AUDIO_BUFFERING;
        LightLock_Unlock(&s->lock);
    }
    bool progress = false;
    int buffer = opendsp_audio_output_free(o);
    if (!s->eof && buffer >= 0) {
        u32 frames = 0;
        s->codec.in.expected = generation;
        Result r = s->codec.ops->read(&s->codec, opendsp_audio_output_data(o, buffer), o->frames, &frames);
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
        if (R_FAILED(r)) { LightLock_Unlock(&s->lock); fail(s, r, generation); return false; }
        if (frames) {
            opendsp_audio_output_queue(o, buffer, frames);
            progress = true;
        } else if (s->looping && s->codec.position) {
            s->looped = true;
            ++s->stats.loops;
            LightLock_Unlock(&s->lock);
            r = s->codec.ops->seek(&s->codec, 0);
            if (R_FAILED(r) && r != OPENDSP_AUDIO_ERR_BUSY) fail(s, r, generation);
            return true;
        } else s->eof = true;
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
            ndspChnSetPaused(o->channel, true);
        } else {
            if (!s->started && (o->queued == o->count || (s->eof && o->queued))) s->started = true;
            if (s->started) {
                ndspChnSetPaused(o->channel, false);
                s->state = s->eof ? OPENDSP_AUDIO_DRAINING : OPENDSP_AUDIO_PLAYING;
            }
        }
    }
    LightLock_Unlock(&s->lock);
    return progress;
}

static void worker_main(void *arg)
{
    audio_worker *w = arg;
    unsigned first = 0;
    while (flag(&running)) {
        bool active = false, progress = false;
        for (unsigned n = 0; n < 24 && flag(&running); ++n) {
            unsigned i = (first + n) % 24;
            LightLock_Lock(&registry_lock);
            opendspAudioStream *s = streams[i];
            if (s && !s->closing && s->group == w->group) ++s->refs;
            else s = NULL;
            LightLock_Unlock(&registry_lock);
            if (!s) continue;
            LightLock_Lock(&s->lock);
            bool needs_work = s->reset || (s->play && !s->paused) || s->control_dirty ||
                s->mix_dirty || s->rate_dirty || s->epoch != current_epoch();
            LightLock_Unlock(&s->lock);
            if (needs_work) {
                active = true;
                u64 start = svcGetSystemTick();
                progress |= step(s);
                publish(s, svcGetSystemTick() - start);
            }
            LightLock_Lock(&registry_lock);
            --s->refs;
            LightLock_Unlock(&registry_lock);
        }
        first = (first + 1) % 24;
        if (!flag(&running)) break;
        if (!progress) {
            if (active) LightEvent_WaitTimeout(&w->event, 5000000);
            else LightEvent_Wait(&w->event);
        }
    }
}

static void join_workers(void)
{
    set_flag(&running, false);
    opendspAudioNotifyFrame();
    for (unsigned i = 0; i < 2; ++i) if (workers[i].thread) {
        threadJoin(workers[i].thread, U64_MAX);
        threadFree(workers[i].thread);
        workers[i].thread = NULL;
    }
}

Result opendspAudioInit(u32 channel_mask)
{
    if (!channel_mask || (channel_mask & ~0xffffffu)) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&lifecycle_lock);
    if (flag(&ready)) {
        Result r = pool == channel_mask ? 0 : OPENDSP_AUDIO_ERR_BUSY;
        LightLock_Unlock(&lifecycle_lock);
        return r;
    }
    for (unsigned i = 0; i < 24; ++i) if (streams[i]) {
        LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_BUSY;
    }
    if (!opendsp_running) { LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_STATE; }
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
    if (priority > 0x18) --priority;
    for (unsigned i = 0; i < 2; ++i) {
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
    for (unsigned i = 0; i < 24; ++i) if (streams[i]) {
        LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_BUSY;
    }
    set_flag(&ready, false);
    join_workers();
    if (hooked) { dspUnhook(&hook_cookie); hooked = false; }
    LightLock_Unlock(&lifecycle_lock);
    return 0;
}

Result opendspAudioOpen(opendspAudioStream **out, const char *path, const opendspAudioConfig *config)
{
    if (!out) return OPENDSP_AUDIO_ERR_ARGUMENT;
    *out = NULL;
    opendspAudioConfig cfg = OPENDSP_AUDIO_CONFIG_INIT;
    if (config) {
        if (config->struct_size < sizeof(cfg)) return OPENDSP_AUDIO_ERR_ARGUMENT;
        cfg = *config;
    }
    if (!path || !*path || cfg.channel < -1 || cfg.channel >= 24 ||
        (cfg.buffer_count && (cfg.buffer_count < 2 || cfg.buffer_count > 8)) ||
        (cfg.buffer_frames && (cfg.buffer_frames < 256 || cfg.buffer_frames > 16384)) ||
        cfg.aac_backend > OPENDSP_AAC_BACKEND_DSP || (cfg.flags & ~OPENDSP_AUDIO_LOOP))
        return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&lifecycle_lock);
    if (!flag(&ready) || flag(&sleeping)) { LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_STATE; }
    int channel = cfg.channel;
    if (channel < 0) {
        for (channel = 0; channel < 24; ++channel) if ((pool & (1u << channel)) && !streams[channel]) break;
    }
    if (channel >= 24 || !(pool & (1u << channel)) || streams[channel]) {
        LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_BUSY;
    }
    opendspAudioStream *s = calloc(1, sizeof(*s));
    Result r = OPENDSP_AUDIO_ERR_MEMORY;
    if (!s) goto done;
    LightLock_Init(&s->lock);
    s->epoch = current_epoch();
    s->command_epoch = s->epoch;
    r = opendsp_audio_codec_open(&s->codec, path, cfg.aac_backend);
    if (R_FAILED(r)) goto fail;
    if (s->epoch != current_epoch() || flag(&sleeping)) { r = OPENDSP_AUDIO_ERR_CONTEXT; goto fail; }
    s->info = s->codec.info;
    s->backend = s->codec.backend;
    s->group = s->info.format == OPENDSP_AUDIO_AAC;
    u32 count = cfg.buffer_count ? cfg.buffer_count : 4;
    u32 frames = cfg.buffer_frames ? cfg.buffer_frames : s->group ? 1024 : 2048;
    r = opendsp_audio_output_open(&s->output, channel, count, frames, &s->info);
    if (R_FAILED(r)) goto fail;
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
    LightLock_Unlock(&registry_lock);
    *out = s;
    opendspAudioNotifyFrame();
    goto done;
fail:
    opendsp_audio_codec_close(&s->codec);
    free(s);
done:
    LightLock_Unlock(&lifecycle_lock);
    return r;
}

static Result command(opendspAudioStream *s, unsigned cmd, u64 frame)
{
    if (!s) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready) || flag(&sleeping)) {
        LightLock_Unlock(&s->lock); return OPENDSP_AUDIO_ERR_STATE;
    }
    if (cmd == 4 && ((s->info.format == OPENDSP_AUDIO_AAC && frame) ||
        (s->info.duration_known && frame > s->info.total_frames))) {
        LightLock_Unlock(&s->lock);
        return s->info.format == OPENDSP_AUDIO_AAC ? OPENDSP_AUDIO_ERR_UNSUPPORTED : OPENDSP_AUDIO_ERR_ARGUMENT;
    }
    if (cmd == 0) {
        if (!s->play) {
            if (s->state == OPENDSP_AUDIO_FINISHED || s->state == OPENDSP_AUDIO_ERROR) s->target = 0;
            s->play = true; s->paused = false; s->reset = true; cancel(s);
            s->command_epoch = current_epoch();
            s->state = OPENDSP_AUDIO_BUFFERING;
        }
    } else if (cmd == 1) {
        if (s->play) { s->paused = true; s->control_dirty = true; s->state = OPENDSP_AUDIO_PAUSED; }
    } else if (cmd == 2) {
        if (s->play) { s->paused = false; s->control_dirty = true; s->state = OPENDSP_AUDIO_BUFFERING; }
    } else {
        s->target = cmd == 3 ? 0 : frame;
        if (cmd == 3) { s->play = false; s->paused = false; }
        s->reset = true; cancel(s);
        s->command_epoch = current_epoch();
        s->state = OPENDSP_AUDIO_STOPPING;
    }
    LightLock_Unlock(&s->lock);
    opendspAudioNotifyFrame();
    return 0;
}
Result opendspAudioPlay(opendspAudioStream *s) { return command(s, 0, 0); }
Result opendspAudioPause(opendspAudioStream *s) { return command(s, 1, 0); }
Result opendspAudioResume(opendspAudioStream *s) { return command(s, 2, 0); }
Result opendspAudioStop(opendspAudioStream *s) { return command(s, 3, 0); }
Result opendspAudioSeek(opendspAudioStream *s, u64 frame) { return command(s, 4, frame); }

Result opendspAudioClose(opendspAudioStream **handle)
{
    if (!handle) return OPENDSP_AUDIO_ERR_ARGUMENT;
    if (!*handle) return 0;
    opendspAudioStream *s = *handle;
    if (LightLock_TryLock(&lifecycle_lock)) return OPENDSP_AUDIO_ERR_BUSY;
    LightLock_Lock(&registry_lock);
    LightLock_Lock(&s->lock);
    s->closing = true;
    s->play = false;
    s->state = OPENDSP_AUDIO_STOPPING;
    cancel(s);
    LightLock_Unlock(&s->lock);
    LightLock_Unlock(&registry_lock);
    opendspAudioNotifyFrame();
    u64 start = svcGetSystemTick();
    for (;;) {
        LightLock_Lock(&registry_lock);
        bool busy = s->refs != 0;
        LightLock_Unlock(&registry_lock);
        if (!busy) {
            opendsp_audio_output_retire(&s->output, current_epoch());
            if (opendsp_audio_output_fenced(&s->output, current_epoch(), flag(&shutdown_complete))) break;
        }
        if (svcGetSystemTick() - start >= SYSCLOCK_ARM11) {
            LightLock_Unlock(&lifecycle_lock); return OPENDSP_AUDIO_ERR_BUSY;
        }
        svcSleepThread(5000000);
    }
    opendsp_audio_codec_close(&s->codec);
    opendsp_audio_output_close(&s->output);
    LightLock_Lock(&registry_lock);
    streams[s->output.channel] = NULL;
    LightLock_Unlock(&registry_lock);
    free(s);
    *handle = NULL;
    LightLock_Unlock(&lifecycle_lock);
    return 0;
}

Result opendspAudioSetLoop(opendspAudioStream *s, bool loop)
{
    if (!s) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) { LightLock_Unlock(&s->lock); return OPENDSP_AUDIO_ERR_STATE; }
    s->looping = loop;
    LightLock_Unlock(&s->lock);
    opendspAudioNotifyFrame(); return 0;
}
Result opendspAudioSetVolume(opendspAudioStream *s, float volume)
{
    if (!s || !isfinite(volume) || volume < 0 || volume > 1) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) { LightLock_Unlock(&s->lock); return OPENDSP_AUDIO_ERR_STATE; }
    s->volume = volume; s->mix_dirty = true;
    LightLock_Unlock(&s->lock);
    opendspAudioNotifyFrame(); return 0;
}
Result opendspAudioSetMix(opendspAudioStream *s, const float mix[12])
{
    if (!s || !mix) return OPENDSP_AUDIO_ERR_ARGUMENT;
    for (unsigned i = 0; i < 12; ++i) if (!isfinite(mix[i]) || mix[i] < -1 || mix[i] > 1) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) { LightLock_Unlock(&s->lock); return OPENDSP_AUDIO_ERR_STATE; }
    memcpy(s->mix, mix, sizeof(s->mix)); s->mix_dirty = true;
    LightLock_Unlock(&s->lock);
    opendspAudioNotifyFrame(); return 0;
}
Result opendspAudioSetRate(opendspAudioStream *s, float rate)
{
    if (!s || !isfinite(rate) || rate < 1 || rate > 4 * 32728) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock);
    if (s->closing || !flag(&ready)) { LightLock_Unlock(&s->lock); return OPENDSP_AUDIO_ERR_STATE; }
    s->rate = rate; s->rate_dirty = true;
    LightLock_Unlock(&s->lock);
    opendspAudioNotifyFrame(); return 0;
}

Result opendspAudioGetInfo(opendspAudioStream *s, opendspAudioInfo *info)
{
    if (!s || !info || info->struct_size < sizeof(*info)) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock); *info = s->info; LightLock_Unlock(&s->lock); return 0;
}
Result opendspAudioGetStats(opendspAudioStream *s, opendspAudioStats *stats)
{
    if (!s || !stats || stats->struct_size < sizeof(*stats)) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock); *stats = s->stats; LightLock_Unlock(&s->lock); return 0;
}
opendspAudioState opendspAudioGetState(opendspAudioStream *s)
{
    if (!s) return OPENDSP_AUDIO_ERROR;
    LightLock_Lock(&s->lock); opendspAudioState value = s->state; LightLock_Unlock(&s->lock); return value;
}
u64 opendspAudioGetPosition(opendspAudioStream *s)
{
    if (!s) return 0;
    LightLock_Lock(&s->lock); u64 value = s->position; LightLock_Unlock(&s->lock); return value;
}
u32 opendspAudioGetBackend(opendspAudioStream *s) { return s ? s->backend : OPENDSP_AAC_BACKEND_AUTO; }
s32 opendspAudioGetChannel(opendspAudioStream *s) { return s ? s->output.channel : -1; }
Result opendspAudioGetLastError(opendspAudioStream *s)
{
    if (!s) return OPENDSP_AUDIO_ERR_ARGUMENT;
    LightLock_Lock(&s->lock); Result value = s->error; LightLock_Unlock(&s->lock); return value;
}

void opendsp_audio_before_exit(void)
{
    LightLock_Lock(&lifecycle_lock);
    set_flag(&ready, false);
    for (unsigned i = 0; i < 24; ++i) if (streams[i]) cancel(streams[i]);
    join_workers();
    if (hooked) { dspUnhook(&hook_cookie); hooked = false; }
    for (unsigned i = 0; i < 24; ++i) if (streams[i]) {
        opendsp_audio_output_retire(&streams[i]->output, current_epoch());
        opendsp_audio_codec_close(&streams[i]->codec);
    }
}

void opendsp_audio_after_exit(bool unloaded)
{
    set_flag(&shutdown_complete, unloaded);
    for (unsigned i = 0; i < 24; ++i) if (streams[i]) {
        LightLock_Lock(&streams[i]->lock);
        streams[i]->state = OPENDSP_AUDIO_ERROR;
        streams[i]->error = OPENDSP_AUDIO_ERR_CONTEXT;
        streams[i]->play = false;
        LightLock_Unlock(&streams[i]->lock);
    }
    LightLock_Unlock(&lifecycle_lock);
}
