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

#define TASK_SP (*(vu16 *)0x0F80)
#define TASK_TOP (*(vu16 *)0x0F83)
#define STACK_WORDS 640
static u16 task_stack[STACK_WORDS];
static u16 z[2048] __attribute__((aligned(4))), tmp[512] __attribute__((aligned(4)));
static u16 pcm[2048] __attribute__((aligned(4)));
static u16 time[4096] __attribute__((section(".aac_time")));
static u16 overlap0[2048] __attribute__((section(".aac_overlap0")));
static u16 overlap1[2048] __attribute__((section(".aac_overlap1")));
static u16 control[32] __attribute__((aligned(4))), result[16] __attribute__((aligned(4)));
static u32 arena, last_frame, frame_ticks, worst_ticks, slice_start, completed_ticks;
static u32 job_start, latency_last, latency_max;
static u16 session, consumed, active, initialize, bytes, table_sum, bad_tables;
static u32 jobs, errors, yields;
static u16 max_stack;
static u32 slice_max;

u32 aac_timer_now(void);
u32 aac_timer_elapsed(u32 start);

static void configure(void)
{
    aac_fb.stage_tw = (u16)(unsigned)d_aac_fft_stage_twiddle;
    aac_fb.imdct_long = (u16)(unsigned)daac_imdct_twiddle_long;
    aac_fb.imdct_short = (u16)(unsigned)daac_imdct_twiddle_short;
    aac_fb.bitrev = (u16)(unsigned)d_aac_head_bitrev;
    aac_fb.sine_long = (u16)(unsigned)daac_sine_long;
    aac_fb.kbd_long = (u16)(unsigned)daac_kbd_long;
    aac_fb.sine_short = (u16)(unsigned)daac_sine_short;
    aac_fb.kbd_short = (u16)(unsigned)daac_kbd_short;
    aac_fb.z = (u16)(unsigned)z; aac_fb.t = (u16)(unsigned)time; aac_fb.tmp = (u16)(unsigned)tmp;
    d_overlap[0] = (u16)(unsigned)overlap0; d_overlap[1] = (u16)(unsigned)overlap1;
    d_pcm = (u16)(unsigned)pcm;
    aac_dsp_init();
}

void aac_stream_init(void)
{
    arena = 0; active = 0; TASK_SP = 0;
    if (dma_error()) dma_quiesce();
}

static void restart(void)
{
    u16 i;
    TASK_SP = 0; active = 0; consumed = 0; initialize = 1;
    jobs = errors = yields = max_stack = slice_max = 0;
    frame_ticks = worst_ticks = completed_ticks = 0;
    latency_last = latency_max = 0;
    for (i = 0; i < STACK_WORDS; i++) task_stack[i] = 0xA55A;
    TASK_TOP = (u16)(unsigned)(task_stack + STACK_WORDS - 1);
    last_frame = aac_timer_now();
}

u16 aac_stream_command(u16 command, vu16 *args, vu16 *reply)
{
    if (command == AAC_CMD_OPEN) {
        u32 base = ((u32)args[0] << 16) | args[1];
        if (args[2] != AAC_ABI || !args[3]) return EXT_ERR_ARGUMENT;
        if ((base & 0x7f) || base < EXT_FCRAM_START || base > EXT_FCRAM_END - AAC_SHARED_BYTES)
            return EXT_ERR_ADDRESS;
        if (arena) return EXT_ERR_ARGUMENT;
        arena = base; session = args[3]; table_sum = args[4];
        restart();
        return EXT_OK;
    }
    if (args[0] != session || !arena) return EXT_ERR_ARGUMENT;
    if (command == AAC_CMD_CLOSE || command == AAC_CMD_RESET) {
        // Acknowledge cancellation only after DMA has stopped.
        active = 0; TASK_SP = 0;
        if (dma_error()) {
            dma_quiesce();
            if (dma_error()) return EXT_ERR_ARGUMENT;
        }
        if (command == AAC_CMD_CLOSE) arena = 0;
        else { session = args[1]; restart(); }
        return EXT_OK;
    }
    if (command == AAC_CMD_STATS) {
        if (args[1]) {
            reply[0] = (u16)(slice_max >> 16); reply[1] = (u16)slice_max;
            return EXT_OK;
        }
        reply[0] = (u16)(jobs >> 16); reply[1] = (u16)jobs;
        reply[2] = (u16)(errors >> 16); reply[3] = (u16)errors;
        reply[4] = (u16)(yields >> 16); reply[5] = (u16)yields;
        reply[6] = max_stack; reply[7] = 0;
        reply[8] = (u16)(completed_ticks >> 16); reply[9] = (u16)completed_ticks;
        reply[10] = (u16)(worst_ticks >> 16); reply[11] = (u16)worst_ticks;
        reply[12] = (u16)(latency_last >> 16); reply[13] = (u16)latency_last;
        reply[14] = (u16)(latency_max >> 16); reply[15] = (u16)latency_max;
        return EXT_OK;
    }
    return EXT_ERR_COMMAND;
}

static void publish(u16 status, u16 changed)
{
    u16 slot = consumed & (AAC_SLOTS - 1), i;
    u32 address = arena + 128 + ((u32)slot << 5);
    for (i = 0; i < 16; i++) result[i] = 0;
    result[0] = (consumed + 1) ^ 0x8000; result[1] = status;
    result[2] = d_channels; result[3] = d_sf_index;
    result[4] = d_sbr_seen; result[5] = changed;
    completed_ticks = frame_ticks + aac_timer_elapsed(slice_start);
    result[6] = (u16)(completed_ticks >> 16); result[7] = (u16)completed_ticks;
    result[8] = session;
    dma_write((u16)(unsigned)result, 16, address);
    result[0] = consumed + 1;
    dma_write((u16)(unsigned)result, 2, address);
    consumed++;
    if (status) errors++; else jobs++;
}

static void write_pcm(void)
{
    u16 at, channels = d_channels, slot = consumed & (AAC_SLOTS - 1);
    void aac_pack_pcm(u16 left, u16 right, u16 dst, u16 channels);
    u32 address = arena + AAC_PCM_OFFSET + ((u32)slot << 12);
    for (at = 0; at < 1024; at += 128) {
        aac_pack_pcm((u16)(unsigned)pcm + at, (u16)(unsigned)pcm + 1024 + at,
                     (u16)(unsigned)tmp, channels);
        dma_write((u16)(unsigned)tmp, channels << 7, address);
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
            for (k = 0; k < AAC_TABLE_WORDS; k++) {
                sum += tables[k];
                if (!(k & 127)) aac_yield();
            }
            bad_tables = sum != table_sum;
            configure(); initialize = 0;
        }
        old_config = d_configured; old_rate = d_sf_index; old_channels = d_channels;
        status = bad_tables ? AAC_BAD_TRANSPORT : aac_dsp_frame(bytes);
        if (!status) write_pcm();
        if (dma_error()) status = AAC_BAD_TRANSPORT;
        // Accumulated active slices exclude mixer service and idle waiting.
        latency_last = aac_timer_elapsed(job_start);
        if (latency_last > latency_max) latency_max = latency_last;
        publish(status, !old_config || old_rate != d_sf_index || old_channels != d_channels);
        active = 0;
        aac_yield();
    }
}

void aac_stream_poll(void)
{
    u32 start, spent;
    u16 i;
    if (!arena) return;
    if (stats.period_last && stats.render_raw_last + stats_aac_used() >= stats.period_last - (stats.period_last >> 3)) return;
    if (!active) {
        // Poll between periods to avoid stalling synchronous ARM submissions.
        u32 now = aac_timer_now();
        u32 interval = stats.period_last ? stats.period_last >> 3 : 81920UL;
        if (aac_timer_delta(last_frame, now) < interval) return;
        last_frame = now;
        dma_read(arena, 32, (u16)(unsigned)control);
        if (dma_error()) return;
        if (control[0] != AAC_MAGIC || control[1] != AAC_ABI || control[2] != session) return;
        if (control[3] == consumed) return;
        if ((u16)(control[3] - consumed) > AAC_SLOTS) { arena = 0; return; }
        frame_ticks = 0; slice_start = aac_timer_now();
        bytes = control[16 + (consumed & 3) * 4];
        if (bytes < 7 || bytes > AAC_INPUT_BYTES) { publish(AAC_UNSUPPORTED, 0); return; }
        if (stats.period_last && stats.render_raw_last > stats.period_last / 2) {
            publish(AAC_RESOURCE_BUSY, 0); return;
        }
        dma_read(arena + AAC_INPUT_OFFSET + ((u32)(consumed & 3) << 11), (bytes + 1) >> 1, (u16)(unsigned)aac_bs_data);
        if (dma_error()) { publish(AAC_BAD_TRANSPORT, 0); return; }
        frame_ticks = 0; active = 1; job_start = aac_timer_now();
    }
    output_poll();
    start = aac_timer_now();
    slice_start = start;
    select_bank(1);
    aac_task_step();
    select_bank(0);
    spent = aac_timer_elapsed(start);
    frame_ticks += spent;
    if (frame_ticks > worst_ticks) worst_ticks = frame_ticks;
    if (spent > slice_max) slice_max = spent;
    stats_charge_aac(spent);
    yields++;
    // Canary and stack high-water mark; exhaustion cancels the task before reuse.
    if (task_stack[0] != 0xA55A || task_stack[1] != 0xA55A) {
        TASK_SP = 0; active = 0; bad_tables = 1; publish(AAC_BAD_TRANSPORT, 0); return;
    }
    if (!active) {
        for (i = 2; i < STACK_WORDS && task_stack[i] == 0xA55A; i++) { }
        if (STACK_WORDS - i > max_stack) max_stack = STACK_WORDS - i;
    }
}
