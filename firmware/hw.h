// SPDX-License-Identifier: BSD-2-Clause
// Teak core and peripheral registers.
#pragma once

typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;
typedef signed long s32;
typedef volatile u16 vu16;

#define FRAME_SAMPLES 160

#define MMIO_BASE 0x7800
#define MMIO(off) (*(vu16 *)(MMIO_BASE + (off)))

// APBP mailbox
#define APBP_REPLY(i)           MMIO(0x0C0 + 4 * (i))
#define APBP_CMD(i)             MMIO(0x0C2 + 4 * (i))
#define APBP_SET_SEM            MMIO(0x0CC)
#define APBP_ACK_SEM            MMIO(0x0D0)
#define APBP_GET_SEM            MMIO(0x0D2)
#define APBP_STAT               MMIO(0x0D6)
#define APBP_STAT_REPLY_FULL(i) (1u << (5 + (i)))
#define APBP_STAT_CMD2          (1u << 13)

// every APBP access needs a short pause
void apbp_delay(void);
static inline u16 apbp_status(void) { u16 v = APBP_STAT; apbp_delay(); return v; }
static inline void apbp_reply(u16 i, u16 v) { APBP_REPLY(i) = v; apbp_delay(); }
static inline u16 apbp_command(u16 i) { u16 v = APBP_CMD(i); apbp_delay(); return v; }
static inline void apbp_raise(u16 v) { APBP_SET_SEM = v; apbp_delay(); }
static inline u16 apbp_semaphores(void) { u16 v = APBP_GET_SEM; apbp_delay(); return v; }
static inline void apbp_clear(u16 v) { APBP_ACK_SEM = v; apbp_delay(); }

// Y window pages the shared memory banks
#define MIU_YPAGE MMIO(0x110)
static inline void select_bank(u16 bank) { MIU_YPAGE = bank; }

// interrupt controller
#define ICU_ACK     MMIO(0x202)
#define ICU_INT0_EN MMIO(0x206)
#define IRQ_BTDMP0  (1u << 0xB)

// audio output (BTDMP0 transmit)
#define BTDMP0_TX_CFG   MMIO(0x2A0)
#define BTDMP0_TX_CLK   MMIO(0x2A2)
#define BTDMP0_TX_EN    MMIO(0x2BE)
#define BTDMP0_TX_STAT  MMIO(0x2C2)
#define BTDMP0_TX_FIFO  MMIO(0x2C6)
#define BTDMP0_TX_FLUSH MMIO(0x2CA)
#define BTDMP_TX_IRQ_EN (1u << 8)
#define BTDMP_TX_FULL   (1u << 3)

void irq_enable(void);
void irq_disable(void);
void irq_unmask(u16 mod3_bits);
