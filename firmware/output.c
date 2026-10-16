// SPDX-License-Identifier: BSD-2-Clause
// BTDMP0 output from a double buffer.
#include "output.h"
#include "stats.h"

volatile u16 frames_due;
volatile u16 render_buffer;
u16 output_running;
static volatile u16 play_buffer, play_position;

vu16 *output_buffer(u16 index)
{
    return (vu16 *)(index ? OUTPUT_BUFFERS + 2 * FRAME_SAMPLES : OUTPUT_BUFFERS);
}

#define BTDMP_TX_EMPTY (1u << 4)

static void fill_fifo(void)
{
    vu16 *buf = output_buffer(play_buffer);
    u16 pos = play_position;
    if (BTDMP0_TX_STAT & BTDMP_TX_EMPTY)
        stats.fifo_empty++;
    while (!(BTDMP0_TX_STAT & BTDMP_TX_FULL)) {
        BTDMP0_TX_FIFO = buf[pos];
        if (++pos == 2 * FRAME_SAMPLES) {
            pos = 0;
            render_buffer = play_buffer;
            play_buffer ^= 1;
            buf = output_buffer(play_buffer);
            frames_due++;
        }
    }
    play_position = pos;
}

// no block repeat here (see crt0.s)
void isr_int0(void)
{
    ICU_ACK = IRQ_BTDMP0;
    fill_fifo();
}

void isr_int1(void) { }

// fallback if the IRQ stalls
void output_poll(void)
{
    if (output_running && !(BTDMP0_TX_STAT & BTDMP_TX_FULL)) {
        irq_disable();
        fill_fifo();
        irq_enable();
    }
}

static void flush_fifo(void)
{
    u16 i;
    BTDMP0_TX_EN = 0;
    BTDMP0_TX_FLUSH = 4;
    for (i = 0; i < 1000 && (BTDMP0_TX_FLUSH & 4); i++) { }
}

void output_start(void)
{
    vu16 *p = output_buffer(0);
    u16 i;
    for (i = 0; i < 4 * FRAME_SAMPLES; i++)
        p[i] = 0;
    play_buffer = 0;
    play_position = 0;
    render_buffer = 1;
    frames_due = 0;

    BTDMP0_TX_CFG = 0x000F;
    BTDMP0_TX_CLK = 0x1004;
    MMIO(0x2A4) = 0x0004;
    MMIO(0x2A6) = 0x0021;
    MMIO(0x2A8) = 0;
    MMIO(0x2AA) = 0;
    MMIO(0x2AC) = 0;
    flush_fifo();
    BTDMP0_TX_CFG = 0x000F | BTDMP_TX_IRQ_EN;
    ICU_ACK = IRQ_BTDMP0;
    ICU_INT0_EN = IRQ_BTDMP0;
    irq_unmask(0x0100);
    fill_fifo();                // IRQ fires only when emptied
    BTDMP0_TX_EN = 0x8000;
    output_running = 1;
    irq_enable();
}

void output_stop(void)
{
    irq_disable();
    output_running = 0;
    flush_fifo();
    BTDMP0_TX_CFG = 0x000F;
    ICU_ACK = IRQ_BTDMP0;
    frames_due = 0;
    irq_enable();
}
