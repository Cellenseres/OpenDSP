// SPDX-License-Identifier: BSD-2-Clause
// Boot handshake, pipes, commands and the frame loop.
#include "hw.h"
#include "shared_mem.h"
#include "output.h"
#include "source.h"
#include "dma.h"
#include "mailbox.h"
#include "stats.h"
#include "aac/stream.h"

#define PIPE_TABLE     0x0800
#define PIPE_BUFFERS   0x0900
#define PIPE_BUF_WORDS 0x20
#define NUM_SLOTS      32

// slot = pipe * 2 + direction
#define SLOT_AUDIO_OUT  4
#define SLOT_AUDIO_IN   5
#define SLOT_BINARY_OUT 6
#define SLOT_BINARY_IN  7

typedef struct { u16 address, size, read, write, flags; } pipe_t;
#define PIPES ((volatile pipe_t *)PIPE_TABLE)

static void pipes_init(void)
{
    u16 i;
    for (i = 0; i < NUM_SLOTS; i++) {
        PIPES[i].address = PIPE_BUFFERS + i * PIPE_BUF_WORDS;
        PIPES[i].size = PIPE_BUF_WORDS * 2;
        PIPES[i].read = 0;
        PIPES[i].write = 0;
        PIPES[i].flags = i;
    }
}

// ring offsets in bytes, bit 15 = wrap
static void pipe_put(u16 slot, u16 value)
{
    volatile pipe_t *p = &PIPES[slot];
    u16 w = p->write;
    vu16 *buf = (vu16 *)(unsigned)p->address;
    if ((w ^ p->read) == 0x8000)
        return;
    buf[(w & 0x7FFF) >> 1] = value;
    w += 2;
    if ((w & 0x7FFF) == p->size)
        w = (w & 0x8000) ^ 0x8000;
    p->write = w;
}

static int pipe_get(u16 slot, u16 *value)
{
    volatile pipe_t *p = &PIPES[slot];
    u16 r = p->read;
    vu16 *buf = (vu16 *)(unsigned)p->address;
    if (r == p->write)
        return 0;
    *value = buf[(r & 0x7FFF) >> 1];
    r += 2;
    if ((r & 0x7FFF) == p->size)
        r = (r & 0x8000) ^ 0x8000;
    p->read = r;
    return 1;
}

// slot notifications on REPLY2, one at a time
#define NOTIFY_QUEUE 8
static u16 notify_queue[NOTIFY_QUEUE], notify_head, notify_count;

static void notify_poll(void)
{
    if (notify_count && !(apbp_status() & APBP_STAT_REPLY_FULL(2))) {
        apbp_reply(2, notify_queue[notify_head]);
        apbp_raise(0x8000);
        notify_head = (notify_head + 1) & (NOTIFY_QUEUE - 1);
        notify_count--;
    }
}

static void notify(u16 slot)
{
    u16 i;
    if (slot == SLOT_AUDIO_OUT)         // frame notifications coalesce
        for (i = 0; i < notify_count; i++)
            if (notify_queue[(notify_head + i) & (NOTIFY_QUEUE - 1)] == slot)
                return;
    if (notify_count == NOTIFY_QUEUE)
        return;
    notify_queue[(notify_head + notify_count) & (NOTIFY_QUEUE - 1)] = slot;
    notify_count++;
    notify_poll();
}

// REPLY0 = 1 answers shutdown and sleep
static u16 reply0_pending;

static void reply0_poll(void)
{
    if (reply0_pending && !(apbp_status() & APBP_STAT_REPLY_FULL(0))) {
        apbp_reply(0, 1);
        reply0_pending = 0;
    }
}

static void unload(void)
{
    u16 n;
    output_stop();
    irq_disable();
    dma_quiesce();
    for (n = 0; n < 60000 && (apbp_status() & APBP_STAT_REPLY_FULL(2)); n++) { }
    apbp_reply(2, 0x8000);
    for (;;) { }
}

// after crt0's handshake: pipe table address
static void send_pipe_table(void)
{
    while (apbp_status() & APBP_STAT_REPLY_FULL(2))
        if ((apbp_status() & APBP_STAT_CMD2) && apbp_command(2) == 0x8000)
            unload();
    apbp_reply(2, PIPE_TABLE);
    apbp_raise(0x8000);
}

static const u16 struct_table[SHM_NUM_STRUCTS] = SHM_STRUCT_TABLE;

static void send_struct_addresses(void)
{
    u16 i;
    pipe_put(SLOT_AUDIO_OUT, SHM_NUM_STRUCTS);
    for (i = 0; i < SHM_NUM_STRUCTS; i++)
        pipe_put(SLOT_AUDIO_OUT, struct_table[i]);
    notify(SLOT_AUDIO_OUT);
}

static void clear_bank(u16 bank)
{
    vu16 *p;
    select_bank(bank);
    for (p = (vu16 *)SHM_DSP_STATUS; p != (vu16 *)SHM_END; p++)
        *p = 0;
    select_bank(0);
}

enum { STATE_OFF, STATE_ON, STATE_SLEEP };
static u16 state;
static u16 frames;

// init answer waits for ARM semaphore 0x4000
#define INIT_WAIT_PASSES 40000u
static u16 init_pending, init_semaphore, init_wait;

static void answer_init_if_ready(void)
{
    if (!init_pending)
        return;
    if (!init_semaphore && --init_wait)
        return;
    init_pending = 0;
    init_semaphore = 0;
    state = STATE_ON;
    send_struct_addresses();
}

static void start_init(void)
{
    init_pending = 1;
    init_wait = INIT_WAIT_PASSES;
}

static void handle_audio_command(void)
{
    u16 cmd, pad, wake_arm = 0;
    while (pipe_get(SLOT_AUDIO_IN, &cmd)) {
        pipe_get(SLOT_AUDIO_IN, &pad);
        switch (cmd & 0xFF) {
        case 0:                         // initialize
            aac_stream_init();
            clear_bank(0);
            clear_bank(1);
            frames = 0;
            sources_init();
            if (!output_running)
                output_start();
            state = STATE_OFF;
            start_init();
            break;
        case 1:                         // shutdown
            aac_stream_init();
            state = STATE_OFF;
            init_pending = 0;
            output_stop();
            reply0_pending = 1;
            break;
        case 2:                         // wake up
            aac_stream_init();
            if (!output_running)
                output_start();
            start_init();
            break;
        case 3:                         // sleep
            aac_stream_init();
            state = STATE_SLEEP;
            init_pending = 0;
            wake_arm = 1;               // releases ndsp's waiting thread
            reply0_pending = 1;
            break;
        }
    }
    notify(SLOT_AUDIO_IN);              // ack first
    if (wake_arm)
        notify(SLOT_AUDIO_OUT);
}

// pipe 3 (AAC): always answer with an error
static u16 message[16], message_len;

static void handle_binary_command(void)
{
    u16 v, i, done = 0;
    while (pipe_get(SLOT_BINARY_IN, &v)) {
        if (message_len < 16)
            message[message_len] = v;
        if (++message_len == 16) {
            pipe_put(SLOT_BINARY_OUT, message[0]);
            pipe_put(SLOT_BINARY_OUT, message[1]);
            pipe_put(SLOT_BINARY_OUT, 1);
            pipe_put(SLOT_BINARY_OUT, 0);
            for (i = 4; i < 16; i++)
                pipe_put(SLOT_BINARY_OUT, 0);
            message_len = 0;
            done = 1;
        }
    }
    notify(SLOT_BINARY_IN);
    if (done)
        notify(SLOT_BINARY_OUT);
}

static void handle_arm_write(u16 slot)
{
    u16 v;
    if (slot == SLOT_AUDIO_IN)
        handle_audio_command();
    else if (slot == SLOT_BINARY_IN)
        handle_binary_command();
    else if (slot < NUM_SLOTS) {
        while (pipe_get(slot, &v)) { }
        notify(slot);
    }
}

static void ipc_poll(void)
{
    u16 sem;
    if (apbp_status() & APBP_STAT_CMD2) {
        u16 v = apbp_command(2);
        if (v == 0x8000)
            unload();
        if (v & 1)
            handle_arm_write(v);
    }
    sem = apbp_semaphores();
    if (sem & 0x4000) {
        apbp_clear(0x4000);
        init_semaphore = 1;
    }
    if (sem & 0x2000)
        apbp_clear(0x2000);
    answer_init_if_ready();
    notify_poll();
    reply0_poll();
}

static u16 frame_counter(u16 bank)
{
    u16 v;
    select_bank(bank);
    v = *(vu16 *)SHM_FRAME_COUNTER;
    select_bank(0);
    return v;
}

// newest frame counter marks the read bank
static u16 read_bank(void)
{
    u16 c0 = frame_counter(0), c1 = frame_counter(1);
    if (c0 == 0xFFFF && c1 != 0xFFFE)
        return 1;
    if (c1 == 0xFFFF && c0 != 0xFFFE)
        return 0;
    return c1 > c0 ? 1 : 0;
}

static void render_frame(void)
{
    vu16 *out = output_buffer(render_buffer);
    u16 i, bank;
    frames++;
    if (state != STATE_ON) {
        for (i = 0; i < 2 * FRAME_SAMPLES; i++)
            out[i] = 0;
        return;
    }
    bank = read_bank();
    sources_render(bank, bank ^ 1, out);
    select_bank(bank ^ 1);
    {
        vu16 *final = (vu16 *)SHM_FINAL_SAMPLES;
        for (i = 0; i < 2 * FRAME_SAMPLES; i++)
            final[i] = out[i];
        ((vu16 *)SHM_DSP_STATUS)[0] = frames;
    }
    select_bank(0);
    notify(SLOT_AUDIO_OUT);
}

int main(void)
{
    state = STATE_OFF;
    aac_stream_init();
    stats_init();
    mailbox_init();
    pipes_init();
    send_pipe_table();
    select_bank(0);
    output_start();
    for (;;) {
        ipc_poll();
        mailbox_poll();
        output_poll();
        if (frames_due) {
            irq_disable();
            frames_due--;
            irq_enable();
            stats_frame_begin(frames_due);
            render_frame();
            stats_frame_end();
        } else aac_stream_poll();
    }
}
