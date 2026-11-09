// SPDX-License-Identifier: BSD-2-Clause
#include "stream.h"
#include "decoder.h"
#include "banks.h"
#include "task.h"
#include "dma.h"
#include "extension.h"
#include "codec_banks_protocol.h"
#include "ogg_protocol.h"
#include "stats.h"

_Static_assert(OGG_DECODE_WORD + OGG_TASK_ERROR < OGG_OVERLAY_WORD, "Task metadata overlap");
_Static_assert(OGG_OVERLAY_DIRECTORY + CODEC_BANK_LIMIT * 4 <= 0x7100, "DMA staging overlap");
_Static_assert(OGG_TASK_GUARD == 0x4600 && OGG_PCM_GUARD == 0x2d60 &&
               OGG_DECODE_WORD + OGG_TASK_ERROR == 0x7016 && OGG_OVERLAY_WORD == 0x7020,
               "Guard kernel addresses changed");
u16 ogg_stream_guard_error(void);

static u16 session;
#define DONE_BOX ((vu16 *)EXT_BASE)
#define DONE_TICKET (DONE_BOX[OGG_DONE_WORD])
static struct {
    u32 banks[4], input, output, output_end;
    u16 channels, initialized, ticket, state, result, frames, duration, range[2];
    u16 failed, started, max_stack;
#ifdef OPENDSP_VORBIS_DSP
    u16 codec;
#endif
    u32 jobs, errors, yields, last_ticks, worst_ticks, slice_max, latency, latency_max, job_ticks, job_started;
} stream __attribute__((section(".codec_meta")));

static u32 timer_now(void)
{
    u16 high, low;
    do {
        high = MMIO(0x03a);
        low = MMIO(0x038);
    } while (high != MMIO(0x03a));
    return ((u32)high << 16) | low;
}

static void reply_u32(vu16 *reply, u32 value)
{
    reply[0] = (u16)(value >> 16);
    reply[1] = (u16)value;
}

static u32 read_u32(const volatile u16 *words)
{
    return ((u32)words[0] << 16) | words[1];
}

static void task_u32(u16 index, u32 value)
{
    ogg_task_write(OGG_DECODE_WORD + index, (u16)(value >> 16));
    ogg_task_write(OGG_DECODE_WORD + index + 1, (u16)value);
}

static u16 task_word(u16 index)
{
    return ogg_task_read(OGG_DECODE_WORD + index);
}

static u16 open_stream(vu16 *args)
{
    u32 begin = read_u32(args + 4), end = read_u32(args + 6);
    u32 input = read_u32(args + 8), output = read_u32(args + 10);
    u16 index, word, channels = args[3];
#ifdef OPENDSP_VORBIS_DSP
    u16 codec = channels & OGG_CODEC_VORBIS;
    channels &= ~OGG_CODEC_VORBIS;
#endif
    if (session || args[0] != OGG_ABI || !args[1] || channels < 1 || channels > 2)
        return EXT_ERR_ARGUMENT;
    if ((begin & 127) || (end & 127) || begin < EXT_FCRAM_START || end > EXT_FCRAM_END || begin >= end ||
        (input & 127) || input < begin || input >= end || OGG_INPUT_BYTES > end - input ||
        (output & 127) || output < begin || output >= end || OGG_PCM_BYTES > end - output ||
        !(output + OGG_PCM_BYTES <= input || input + OGG_INPUT_BYTES <= output)) return EXT_ERR_ADDRESS;
    if (codec_bank_claim(args[2], stream.banks)) return EXT_ERR_ARGUMENT;
    if (!(end <= stream.banks[0] || begin >= stream.banks[1]) || stream.banks[3] > CODEC_BANK_LIMIT)
        return EXT_ERR_ADDRESS;
    for (index = 0; index < (u16)stream.banks[3]; ++index) {
        const u16 *descriptor = dma_fetch_words(stream.banks[2] + (u32)index * 8, 4);
        if (dma_error()) return EXT_ERR_ADDRESS;
        for (word = 0; word < 4; ++word)
            ogg_task_write(OGG_OVERLAY_DIRECTORY + index * 4 + word, descriptor[word]);
    }
    for (index = 0; index < 32; ++index) ogg_task_write(OGG_DECODE_WORD + index, 0);
    for (index = 0; index < 4; ++index)
        ogg_task_write(OGG_OVERLAY_CONFIG + index, (u16)(stream.banks[index / 2] >> (index & 1 ? 0 : 16)));
    ogg_task_write(OGG_OVERLAY_CONFIG + 4, (u16)stream.banks[3]);
    ogg_task_write(OGG_OVERLAY_WORD, 0);
    stream.input = input;
    stream.output = output;
    stream.output_end = output + OGG_PCM_BYTES;
    stream.channels = channels;
#ifdef OPENDSP_VORBIS_DSP
    stream.codec = codec;
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_CODEC, codec);
#endif
    stream.initialized = stream.ticket = stream.failed = stream.started = stream.result = stream.frames = 0;
    stream.duration = stream.range[0] = stream.range[1] = 0;
    stream.state = OGG_STATE_PREPARED;
    stream.max_stack = 0;
    stream.jobs = stream.errors = stream.yields = stream.last_ticks = stream.worst_ticks = stream.slice_max = 0;
    stream.latency = stream.latency_max = stream.job_ticks = stream.job_started = 0;
    OGG_TASK_CONTEXT[0] = 0;
    OGG_TASK_CONTEXT[3] = OGG_TASK_TOP;
    task_u32(OGG_TASK_PACKET, input);
    task_u32(OGG_TASK_INPUT_BEGIN, input);
    task_u32(OGG_TASK_INPUT_END, input + OGG_INPUT_BYTES);
    task_u32(OGG_TASK_PCM, output);
    task_u32(OGG_TASK_PCM_END, stream.output_end);
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_CHANNELS, stream.channels);
    session = args[1];
    return EXT_OK;
}

static void publish_done(void)
{
    vu16 *status = DONE_BOX + OGG_DONE_STATUS;
    status[0] = stream.state;
    status[1] = stream.result;
    status[2] = stream.frames;
    status[3] = stream.channels;
    DONE_TICKET = stream.ticket;
}

u16 ogg_stream_busy(void)
{
    return session != 0;
}

void ogg_stream_cancel(void)
{
    if (!session) return;
    if (stream.started && task_word(OGG_TASK_ERROR) && ogg_stream_quiesce()) return;
    OGG_TASK_CONTEXT[0] = 0;
    session = 0;
}

u16 ogg_stream_command(u16 command, vu16 *args, vu16 *reply, u16 busy)
{
    if (command == OGG_CMD_OPEN) return busy ? EXT_ERR_ARGUMENT : open_stream(args);
    if (command == OGG_CMD_CLOSE && !session) return EXT_OK;
    if (!session || args[0] != session) return EXT_ERR_ARGUMENT;
    if (command == OGG_CMD_STATS) {
        if (args[1]) {
            reply_u32(reply, stream.slice_max);
            return EXT_OK;
        }
        reply_u32(reply, stream.jobs);
        reply_u32(reply + 2, stream.errors);
        reply_u32(reply + 4, stream.yields);
        reply[6] = stream.max_stack;
        reply[7] = 0;
        reply_u32(reply + 8, stream.last_ticks);
        reply_u32(reply + 10, stream.worst_ticks);
        reply_u32(reply + 12, stream.latency);
        reply_u32(reply + 14, stream.latency_max);
        return EXT_OK;
    }
    if (command == OGG_CMD_STATUS) {
        reply[0] = stream.ticket;
        reply[1] = stream.state;
        reply[2] = stream.result;
        reply[3] = stream.frames;
        reply[4] = stream.channels;
        reply[5] = stream.duration;
        reply[6] = stream.range[0];
        reply[7] = stream.range[1];
        return EXT_OK;
    }
    if (command == OGG_CMD_CLOSE) {
        if (stream.started && task_word(OGG_TASK_ERROR) && ogg_stream_quiesce()) return EXT_ERR_ADDRESS;
        ogg_stream_cancel();
        return EXT_OK;
    }
    if (stream.failed || stream.state == OGG_STATE_BUSY || !args[1] || args[1] == stream.ticket)
        return EXT_ERR_ARGUMENT;
    if (command != OGG_CMD_INIT && command != OGG_CMD_DECODE
#ifdef OPENDSP_VORBIS_DSP
        && command != OGG_CMD_RESET
#endif
       ) return EXT_ERR_COMMAND;
#ifdef OPENDSP_VORBIS_DSP
    if (command == OGG_CMD_RESET && (!stream.codec || !stream.initialized)) return EXT_ERR_ARGUMENT;
    if (command == OGG_CMD_DECODE && stream.codec && (!args[2] || args[3] > 4096)) return EXT_ERR_ARGUMENT;
#endif
    if (command == OGG_CMD_DECODE && (!stream.initialized || !args[3] || args[3] > OGG_PCM_FRAMES || args[4] > 1))
        return EXT_ERR_ARGUMENT;
    stream.ticket = args[1];
    stream.result = stream.frames = 0;
    stream.job_ticks = 0;
    stream.job_started = timer_now();
    stream.state = OGG_STATE_BUSY;
    DONE_TICKET = 0;
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_OPERATION, command);
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_BYTES, args[2]);
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_FRAMES, args[3]);
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_FEC, args[4]);
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_GAIN, args[5]);
#ifdef OPENDSP_VORBIS_DSP
    if (stream.codec)
        for (u16 index = 0; index < 4; ++index)
            ogg_task_write(OGG_DECODE_WORD + OGG_TASK_GRANULE + index, args[6 + index]);
#endif
    ogg_task_write(OGG_DECODE_WORD + OGG_TASK_STATUS, OGG_TASK_RUNNING);
    return EXT_OK;
}

void ogg_stream_poll(void)
{
    u16 failed;
    u32 started, spent;
    if (!session || stream.state != OGG_STATE_BUSY) return;
    stream.started = 1;
    started = timer_now();
    ogg_task_step();
    if (OGG_TASK_CONTEXT[0] <= OGG_TASK_TOP && OGG_TASK_CONTEXT[0] >= OGG_TASK_GUARD + 32) {
        u16 used = OGG_TASK_TOP - OGG_TASK_CONTEXT[0];
        if (used > stream.max_stack) stream.max_stack = used;
    }
    failed = ogg_stream_guard_error();
    spent = started - timer_now();
    stream.job_ticks += spent;
    if (spent > stream.slice_max) stream.slice_max = spent;
    stats_charge_aac(spent);
    stream.yields++;
    if (failed) {
        stream.failed = 1;
        stream.errors++;
        stream.state = OGG_STATE_FAILED;
        publish_done();
        void codec_notify(void);
        codec_notify();
        return;
    }
    if (task_word(OGG_TASK_STATUS) != OGG_TASK_COMPLETE) return;
    stream.result = task_word(OGG_TASK_RESULT);
    stream.frames = task_word(OGG_TASK_DECODED);
    stream.duration = task_word(OGG_TASK_DURATION);
    stream.range[0] = task_word(OGG_TASK_RANGE);
    stream.range[1] = task_word(OGG_TASK_RANGE + 1);
    if (task_word(OGG_TASK_OPERATION) == OGG_CMD_INIT) stream.initialized = stream.result == 0;
    else {
#ifdef OPENDSP_VORBIS_DSP
        if (task_word(OGG_TASK_OPERATION) == OGG_CMD_RESET) {
            if (stream.result) stream.errors++;
        } else if ((s16)stream.result > 0 || (stream.codec && !stream.result)) stream.jobs++;
#else
        if ((s16)stream.result > 0) stream.jobs++;
#endif
        else stream.errors++;
    }
    stream.last_ticks = stream.job_ticks;
    if (stream.last_ticks > stream.worst_ticks) stream.worst_ticks = stream.last_ticks;
    stream.latency = stream.job_started - timer_now();
    if (stream.latency > stream.latency_max) stream.latency_max = stream.latency;
    stream.state = OGG_STATE_DONE;
    publish_done();
    void codec_notify(void);
    codec_notify();
}
