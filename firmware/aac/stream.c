// SPDX-License-Identifier: BSD-2-Clause
// Main-loop cooperative decoder. Ring addresses derive only from a validated arena.
#include "stream.h"
#include "aac_dsp.h"
#include "aac_fb.h"
#include "aac_tables_dsp.h"
#include "aac_protocol.h"
#include "extension.h"
#include "dma.h"
#include "output.h"
#include "stats.h"
#include "mp3/decoder.h"
#include "mp3_protocol.h"

#define TASK_SP (*(vu16 *)0x0F80)
#define TASK_TOP (*(vu16 *)0x0F83)
#define STACK_WORDS 640
static u16 task_stack[STACK_WORDS];
/* One compressed-codec lease, with exactly the old AAC scratch footprint. */
static u16 codec_work[4608] __attribute__((aligned(4)));
#define AAC_Z codec_work
#define AAC_PCM (codec_work + 2048)
#define AAC_TMP (codec_work + 4096)
static u16 time[4096] __attribute__((section(".aac_time"), aligned(4)));
static u16 overlap0[2048] __attribute__((section(".aac_overlap0")));
static u16 overlap1[2048] __attribute__((section(".aac_overlap1")));
static u16 control[32] __attribute__((aligned(4))), result[16] __attribute__((aligned(4)));
static u32 arena, last_frame, frame_ticks, worst_ticks, slice_start, completed_ticks;
static u32 job_start, latency_last, latency_max;
static u16 session, consumed, active, initialize, bytes, table_sum, bad_tables;
static u32 jobs, errors, yields;
static u16 max_stack;
static u32 slice_max;
static u16 codec, pcm_frames;
static u16 codec_channels, codec_rate;
static u32 mp3_slice_start;
static u32 mp3_yield_deadline;
static u16 mp3_yield_fast;
static u16 stream_mode, stream_frames, stream_count, stream_waiting, stream_profile, stream_play_pending,
    stream_play_ack;
static u16 stream_started;
static u32 stream_pcm_begin, stream_pcm_end;
void codec_notify(void);
static mp3_decoder mp3;
u16 *mp3_pcm_buffer(mp3_decoder *d)
{
    (void)d;
    return codec_work + 4480;
}
#define MP3_STATE ((mp3_state *)time)
#define MP3_STREAM_LAST_TICKS ((u32 *)(time + 4084))
typedef char mp3_state_fits[(sizeof(mp3_state) <= 3072 * sizeof(u16)) ? 1 : -1];

u32 aac_timer_now(void);
u32 aac_timer_elapsed(u32 start);

static void mp3_charge_stage(void)
{
    u32 now = aac_timer_now();
    MP3_STATE->stage_ticks[MP3_STATE->stage] += aac_timer_delta(mp3_slice_start, now);
    mp3_slice_start = now;
}
void mp3_codec_phase(mp3_state *s, u16 stage)
{
    if (!stream_mode || stream_profile) {
        mp3_charge_stage();
        s->stage = stage;
    }
}
static __attribute__((noinline)) void mp3_codec_yield_slow(void)
{
    /* MP3 yields at 1/8 period; admission reserves 7/8 for mixer work. */
    if (stream_mode && !mp3_yield_fast && !frames_due) {
        u32 period = stats.period_last ? stats.period_last : 655360UL;
        u32 spent = aac_timer_elapsed(slice_start);
        if (spent < (period >> 2) &&
            stats.render_raw_last + stats_aac_used() + spent < period - (period >> 3))
            return;
    }
    if (!stream_mode || stream_profile) {
        mp3_charge_stage();
        aac_yield();
        mp3_slice_start = aac_timer_now();
    } else
        aac_yield();
}

u16 mp3_slice_ready(const u32 *deadline, const volatile u16 *due);
void mp3_codec_yield(void)
{
    if (mp3_yield_fast && mp3_slice_ready(&mp3_yield_deadline, &frames_due))
        return;
    mp3_codec_yield_slow();
}

static void configure(void)
{
    if (codec) {
        mp3.s = MP3_STATE;
        mp3.spectrum = (s32 *)d_spec;
        mp3.work = (s32 *)codec_work;
        mp3.overlap[0] = overlap0;
        mp3.overlap[1] = overlap1;
        mp3.input = aac_bs_data;
        mp3.main_data = overlap0 + 576;
        mp3_reset(&mp3);
        codec_channels = codec_rate = 0;
        return;
    }
    aac_fb.stage_tw = (u16)(unsigned)d_aac_fft_stage_twiddle;
    aac_fb.imdct_long = (u16)(unsigned)daac_imdct_twiddle_long;
    aac_fb.imdct_short = (u16)(unsigned)daac_imdct_twiddle_short;
    aac_fb.bitrev = (u16)(unsigned)d_aac_head_bitrev;
    aac_fb.sine_long = (u16)(unsigned)daac_sine_long;
    aac_fb.kbd_long = (u16)(unsigned)daac_kbd_long;
    aac_fb.sine_short = (u16)(unsigned)daac_sine_short;
    aac_fb.kbd_short = (u16)(unsigned)daac_kbd_short;
    aac_fb.z = (u16)(unsigned)AAC_Z;
    aac_fb.t = (u16)(unsigned)time;
    aac_fb.tmp = (u16)(unsigned)AAC_TMP;
    d_overlap[0] = (u16)(unsigned)overlap0;
    d_overlap[1] = (u16)(unsigned)overlap1;
    d_pcm = (u16)(unsigned)AAC_PCM;
    aac_dsp_init();
}

void aac_stream_init(void)
{
    arena = 0;
    active = 0;
    TASK_SP = 0;
    stream_mode = stream_waiting = stream_play_pending = 0;
    if (dma_error())
        dma_quiesce();
}

static void restart(u16 counters, u16 preserve_playback)
{
    u16 i;
    TASK_SP = 0;
    active = 0;
    consumed = 0;
    initialize = 1;
    stream_waiting = 0;
    if (!preserve_playback)
        stream_play_pending = stream_play_ack = stream_started = 0;
    if (counters) {
        jobs = errors = yields = max_stack = slice_max = worst_ticks = latency_max = completed_ticks =
            latency_last = 0;
    }
    frame_ticks = 0;
    for (i = 0; i < STACK_WORDS; i++)
        task_stack[i] = 0xA55A;
    TASK_TOP = (u16)(unsigned)(task_stack + STACK_WORDS - 1);
    last_frame = aac_timer_now();
}

u16 aac_stream_command(u16 command, vu16 *args, vu16 *reply)
{
    u16 byte_stream = command == MP3_CMD_STREAM_OPEN;
    u16 requested = command >= MP3_CMD_OPEN && command <= MP3_CMD_STREAM_OPEN;
    u32 shared = requested ? MP3_SHARED_BYTES : AAC_SHARED_BYTES;
    if (byte_stream)
        command = AAC_CMD_OPEN;
    else if (requested)
        command -= MP3_CMD_OPEN - AAC_CMD_OPEN;
    if (command == AAC_CMD_OPEN) {
        u32 base = ((u32)args[0] << 16) | args[1];
        if (args[2] != (byte_stream ? MP3_STREAM_ABI : requested ? MP3_ABI : AAC_ABI) || !args[3])
            return EXT_ERR_ARGUMENT;
        if (byte_stream) {
            /* Two decoded frames remain queued while one slot is being filled. */
            static const u16 minimum[9] = {0, 0, 0, 1152, 768, 576, 461, 384, 330};
            if (args[5] < 3 || args[5] > 8 || args[6] < 256 || args[6] > 16384 || args[7] > 1)
                return EXT_ERR_ARGUMENT;
            if (args[6] < minimum[args[5]])
                return EXT_ERR_ARGUMENT;
            shared = MP3_STREAM_PCM + mp3_slot_bytes(args[5], args[6]);
        }
        if ((base & 0x7f) || base < EXT_FCRAM_START || base > EXT_FCRAM_END - shared)
            return EXT_ERR_ADDRESS;
        if (arena)
            return EXT_ERR_ARGUMENT;
        arena = base;
        session = args[3];
        table_sum = args[4];
        codec = requested;
        stream_mode = byte_stream;
        if (byte_stream) {
            stream_count = args[5];
            stream_frames = args[6];
            stream_profile = args[7];
            stream_pcm_begin = base + MP3_STREAM_PCM;
            stream_pcm_end = base + shared;
            *(vu16 *)MP3_STREAM_META_WORD = 0;
        }
        restart(1, 0);
        if (byte_stream)
            stream_waiting = 1;
        return EXT_OK;
    }
    if (requested != codec || args[0] != session || !arena)
        return EXT_ERR_ARGUMENT;
    if (command == AAC_CMD_CLOSE || command == AAC_CMD_RESET) {
        if (command == AAC_CMD_RESET && stream_mode && (!args[1] || args[2] > 1))
            return EXT_ERR_ARGUMENT;
        // Acknowledge cancellation only after DMA has stopped.
        active = 0;
        TASK_SP = 0;
        if (dma_error()) {
            dma_quiesce();
            if (dma_error())
                return EXT_ERR_ARGUMENT;
        }
        if (command == AAC_CMD_CLOSE)
            arena = 0;
        else {
            session = args[1];
            restart(!stream_mode, stream_mode && args[2]);
            if (stream_mode) {
                *(vu16 *)MP3_STREAM_META_WORD = 0;
                stream_waiting = 1;
            }
        }
        return EXT_OK;
    }
    if (command == AAC_CMD_STATS) {
        if (codec && args[1] == 2) {
            u16 i;
            select_bank(1);
            for (i = 0; i < 6; ++i) {
                u32 ticks =
                    stream_mode ? (jobs ? MP3_STREAM_LAST_TICKS[i] : 0) : MP3_STATE->last_stage_ticks[i];
                reply[2 * i] = (u16)(ticks >> 16);
                reply[2 * i + 1] = (u16)ticks;
            }
            select_bank(0);
            return EXT_OK;
        }
        if (args[1]) {
            reply[0] = (u16)(slice_max >> 16);
            reply[1] = (u16)slice_max;
            return EXT_OK;
        }
        reply[0] = (u16)(jobs >> 16);
        reply[1] = (u16)jobs;
        reply[2] = (u16)(errors >> 16);
        reply[3] = (u16)errors;
        reply[4] = (u16)(yields >> 16);
        reply[5] = (u16)yields;
        reply[6] = max_stack;
        reply[7] = 0;
        reply[8] = (u16)(completed_ticks >> 16);
        reply[9] = (u16)completed_ticks;
        reply[10] = (u16)(worst_ticks >> 16);
        reply[11] = (u16)worst_ticks;
        reply[12] = (u16)(latency_last >> 16);
        reply[13] = (u16)latency_last;
        reply[14] = (u16)(latency_max >> 16);
        reply[15] = (u16)latency_max;
        return EXT_OK;
    }
    return EXT_ERR_COMMAND;
}

static void publish(u16 status, u16 changed)
{
    u16 slot = consumed & (AAC_SLOTS - 1), i;
    u32 address = arena + 128 + ((u32)slot << 5);
    for (i = 0; i < 16; i++)
        result[i] = 0;
    result[0] = (consumed + 1) ^ 0x8000;
    result[1] = status;
    result[2] = codec ? codec_channels : d_channels;
    result[3] = codec ? codec_rate : d_sf_index;
    result[4] = codec ? 0 : d_sbr_seen;
    result[5] = changed;
    completed_ticks = frame_ticks + aac_timer_elapsed(slice_start);
    result[6] = (u16)(completed_ticks >> 16);
    result[7] = (u16)completed_ticks;
    result[8] = session;
    if (codec && !status)
        result[9] = pcm_frames;
    dma_write((u16)(unsigned)result, 16, address);
    result[0] = consumed + 1;
    dma_write((u16)(unsigned)result, 2, address);
    consumed++;
    if (status)
        errors++;
    else
        jobs++;
    codec_notify();
}

#include "mp3_stream.inc"

void mp3_stream_emit(const u16 *samples, u16 frames)
{
    if (stream_mode) {
        byte_stream_emit(samples, frames);
        return;
    }
    u16 count = MP3_STATE->channels == 2 ? frames << 1 : frames;
    u16 *stage = codec_work + 4480;
    void copy_words(const u16 *, s16 *, u16);
    u32 address = arena + AAC_PCM_OFFSET + mp3_index(consumed & 3, MP3_PCM_BYTES);
    if (frames != 64 || pcm_frames > 1152 - frames || count > 128) {
        bad_tables = 1;
        return;
    }
    address += (u32)pcm_frames << (MP3_STATE->channels == 2 ? 2 : 1);
    /* Synthesis targets X directly. DMA never reads a banked Y alias. */
    (void)samples; /* synthesis writes this X staging area directly */
    dma_write((u16)(unsigned)stage, count, address);
    pcm_frames += frames;
}

static void write_pcm(void)
{
    u16 at, channels = d_channels, slot = consumed & (AAC_SLOTS - 1);
    void aac_pack_pcm(u16 left, u16 right, u16 dst, u16 channels);
    u32 address = arena + AAC_PCM_OFFSET + ((u32)slot << 12);
    for (at = 0; at < 1024; at += 128) {
        aac_pack_pcm((u16)(unsigned)AAC_PCM + at, (u16)(unsigned)AAC_PCM + 1024 + at, (u16)(unsigned)AAC_TMP,
                     channels);
        dma_write((u16)(unsigned)AAC_TMP, channels << 7, address);
        address += (u32)channels << 8;
        aac_yield();
    }
}

void aac_task_main(void)
{
    for (;;) {
        u16 status, old_config, old_rate, old_channels;
        if (initialize) {
            u16 k, sum = 0;
            const u16 *tables = (const u16 *)0xC000;
            for (k = 0; k < (codec ? MP3_TABLE_WORDS : AAC_TABLE_WORDS); k++) {
                sum += tables[k];
                if (!(k & 127))
                    aac_yield();
            }
            bad_tables = sum != table_sum;
            configure();
            initialize = 0;
            if (stream_mode)
                byte_stream_init();
        }
        if (stream_mode) {
            if (bad_tables)
                byte_stream_fail(AAC_BAD_TRANSPORT);
            byte_stream_task();
            active = 0;
            aac_yield();
            continue;
        }
        old_config = codec ? MP3_STATE->configured : d_configured;
        old_rate = codec ? MP3_STATE->rate_index : d_sf_index;
        old_channels = codec ? MP3_STATE->channels : d_channels;
        pcm_frames = 0;
        if (codec) {
            u16 i;
            for (i = 0; i < 6; ++i)
                MP3_STATE->stage_ticks[i] = 0;
            MP3_STATE->stage = 0;
            mp3_slice_start = aac_timer_now();
        }
        status = bad_tables ? AAC_BAD_TRANSPORT : (codec ? mp3_frame(&mp3, bytes) : aac_dsp_frame(bytes));
        if (codec) {
            u16 i;
            mp3_charge_stage();
            for (i = 0; i < 6; ++i)
                MP3_STATE->last_stage_ticks[i] = MP3_STATE->stage_ticks[i];
            codec_channels = MP3_STATE->channels;
            codec_rate = MP3_STATE->rate_index;
        }
        if (!status && !codec)
            write_pcm();
        if (!status && codec && pcm_frames != 1152)
            status = AAC_BAD_TRANSPORT;
        if (dma_error())
            status = AAC_BAD_TRANSPORT;
        // Accumulated active slices exclude mixer service and idle waiting.
        latency_last = aac_timer_elapsed(job_start);
        if (latency_last > latency_max)
            latency_max = latency_last;
        publish(status, !old_config || old_rate != (codec ? MP3_STATE->rate_index : d_sf_index) ||
                            old_channels != (codec ? MP3_STATE->channels : d_channels));
        active = 0;
        aac_yield();
    }
}

void aac_stream_poll(void)
{
    u32 start, spent;
    u16 i;
    if (!arena)
        return;
    if (stream_mode && stream_waiting)
        return;
    if (stats.period_last &&
        stats.render_raw_last + stats_aac_used() >= stats.period_last - (stats.period_last >> 3))
        return;
    if (!active) {
        if (stream_mode) {
            vu16 *meta = (vu16 *)MP3_STREAM_META_WORD;
            if ((meta[31] & 0x8000) || meta[0] != MP3_MAGIC || meta[1] != MP3_STREAM_ABI ||
                meta[2] != session || (!meta[3] && !meta[5])) {
                stream_waiting = 1;
                return;
            }
            frame_ticks = 0;
            active = 1;
            job_start = aac_timer_now();
        } else {
            // Poll between periods to avoid stalling synchronous ARM submissions.
            u32 now = aac_timer_now();
            u32 interval = stats.period_last ? stats.period_last >> 3 : 81920UL;
            if (aac_timer_delta(last_frame, now) < interval)
                return;
            last_frame = now;
            dma_read(arena, 32, (u16)(unsigned)control);
            if (dma_error())
                return;
            if (control[0] != (codec ? MP3_MAGIC : AAC_MAGIC) || control[1] != (codec ? MP3_ABI : AAC_ABI) ||
                control[2] != session)
                return;
            if (control[3] == consumed)
                return;
            if ((u16)(control[3] - consumed) > AAC_SLOTS) {
                arena = 0;
                return;
            }
            frame_ticks = 0;
            slice_start = aac_timer_now();
            bytes = control[16 + (consumed & 3) * 4];
            if (bytes < (codec ? 4 : 7) || bytes > AAC_INPUT_BYTES) {
                publish(AAC_UNSUPPORTED, 0);
                return;
            }
            if (stats.period_last && stats.render_raw_last > stats.period_last / 2) {
                publish(AAC_RESOURCE_BUSY, 0);
                return;
            }
            dma_read(arena + AAC_INPUT_OFFSET + ((u32)(consumed & 3) << 11), (bytes + 1) >> 1,
                     (u16)(unsigned)aac_bs_data);
            if (dma_error()) {
                publish(AAC_BAD_TRANSPORT, 0);
                return;
            }
            frame_ticks = 0;
            active = 1;
            job_start = aac_timer_now();
        }
    }
    output_poll();
    start = aac_timer_now();
    slice_start = start;
    mp3_yield_fast = codec && stream_mode && !stream_profile;
    if (mp3_yield_fast) {
        u32 period = stats.period_last ? stats.period_last : 655360UL;
        u32 left = period - (period >> 3) - stats.render_raw_last - stats_aac_used();
        u32 quantum = period >> 3;
        if (left < quantum)
            quantum = left;
        /* Native modulo-32 subtraction normalizes before the 32-bit spill. */
        mp3_yield_deadline = aac_timer_delta(start, quantum);
    }
    select_bank(1);
    aac_task_step();
    select_bank(0);
    spent = aac_timer_elapsed(start);
    frame_ticks += spent;
    if (frame_ticks > worst_ticks)
        worst_ticks = frame_ticks;
    if (spent > slice_max)
        slice_max = spent;
    stats_charge_aac(spent);
    yields++;
    // Canary and stack high-water mark; exhaustion cancels the task before reuse.
    if (task_stack[0] != 0xA55A || task_stack[1] != 0xA55A) {
        TASK_SP = 0;
        active = 0;
        bad_tables = 1;
        if (stream_mode) {
            stream_waiting = 1;
            select_bank(1);
            BS->flags |= MP3_STREAM_ERROR;
            BS->error = AAC_BAD_TRANSPORT;
            errors++;
            byte_stream_status();
            select_bank(0);
        } else
            publish(AAC_BAD_TRANSPORT, 0);
        return;
    }
    if (!active || (stream_mode && stream_waiting)) {
        for (i = 2; i < STACK_WORDS && task_stack[i] == 0xA55A; i++) {
        }
        if (STACK_WORDS - i > max_stack)
            max_stack = STACK_WORDS - i;
    }
}
