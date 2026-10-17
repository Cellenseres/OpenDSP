// SPDX-License-Identifier: BSD-2-Clause
// FCRAM reads and writes.
#include "dma.h"
#include "voice.h"

#define AHBM_STAT      MMIO(0x0E0)
#define AHBM_CFG0(n)   MMIO(0x0E2 + 6 * (n))
#define AHBM_CFG1(n)   MMIO(0x0E4 + 6 * (n))
#define AHBM_DMAMAP(n) MMIO(0x0E6 + 6 * (n))
#define DMA_END        MMIO(0x18C)
#define DMA_CHAN       MMIO(0x1BE)
#define DMA_SRC_LO     MMIO(0x1C0)
#define DMA_SRC_HI     MMIO(0x1C2)
#define DMA_DST_LO     MMIO(0x1C4)
#define DMA_DST_HI     MMIO(0x1C6)
#define DMA_SIZE0      MMIO(0x1C8)
#define DMA_SIZE1      MMIO(0x1CA)
#define DMA_SIZE2      MMIO(0x1CC)
#define DMA_SSTEP0     MMIO(0x1CE)
#define DMA_DSTEP0     MMIO(0x1D0)
#define DMA_SSTEP1     MMIO(0x1D2)
#define DMA_DSTEP1     MMIO(0x1D4)
#define DMA_SSTEP2     MMIO(0x1D6)
#define DMA_DSTEP2     MMIO(0x1D8)
#define DMA_SPACE      MMIO(0x1DA)
#define DMA_CFG2       MMIO(0x1DC)
#define DMA_CTRL       MMIO(0x1DE)

#define READ_AHBM  2
#define READ_DMA   2
#define WRITE_AHBM 1
#define WRITE_DMA  3

#define BURST_BYTES   32u
#define STAGING_BYTES 2688u
#define CHUNK_BYTES   (STAGING_BYTES - 2 * BURST_BYTES)
#define SPIN_LIMIT    30000u

static u16 staging[STAGING_BYTES / 2] __attribute__((aligned(4)));
static u16 initialized;
static u16 stuck_channels;

u16 dma_error(void)
{
    return stuck_channels;
}

void pcm8_expand(const u16 *src, s16 *dst, u16 pairs);
void copy_words(const u16 *src, s16 *dst, u16 count);

static void wait_ahbm(u16 mask)
{
    u16 n;
    for (n = 0; n < SPIN_LIMIT && (AHBM_STAT & mask); n++) { }
    if (n == SPIN_LIMIT) stuck_channels |= 0x8000;
}

static void wait_channel(u16 ch)
{
    u16 n, bit = (u16)(1u << ch);
    for (n = 0; n < SPIN_LIMIT; n++)
        if (DMA_END & bit)
            break;
    if (n == SPIN_LIMIT)
        stuck_channels |= bit;
    else
        stuck_channels &= (u16)~bit;
    wait_ahbm(0x4);
}

void dma_quiesce(void)
{
    u16 ch;
    for (ch = 0; ch < 8; ch++) {
        if (!(stuck_channels & (1u << ch)))
            continue;
        wait_channel(ch);
        DMA_CHAN = ch;
        DMA_CTRL = 0x8000;
    }
    wait_ahbm(0xF);
}

static void setup_channel(u16 ch, u16 src_step, u16 dst_step)
{
    DMA_CHAN = ch;
    DMA_DST_HI = 0;
    DMA_SIZE2 = 0;
    DMA_SSTEP0 = src_step;
    DMA_DSTEP0 = dst_step;
    DMA_SSTEP1 = src_step;
    DMA_DSTEP1 = dst_step;
    DMA_SSTEP2 = 0;
    DMA_DSTEP2 = 0;
    DMA_CFG2 = 0x0700;
    DMA_SPACE = 0;
}

static void dma_init(void)
{
    wait_ahbm(0xF);

    setup_channel(READ_DMA, 4, 2);
    DMA_SIZE0 = 0x10;                   // one burst per row
    AHBM_DMAMAP(READ_AHBM) = 1u << READ_DMA;
    AHBM_CFG0(READ_AHBM) = 0x0025;      // 32-bit, burst x8
    AHBM_CFG1(READ_AHBM) = 0x0200;
    DMA_SPACE = 0x0607;                 // FCRAM to DSP data

    setup_channel(WRITE_DMA, 2, 4);
    AHBM_DMAMAP(WRITE_AHBM) = 1u << WRITE_DMA;
    AHBM_CFG0(WRITE_AHBM) = 0x0020;     // 32-bit, single
    AHBM_CFG1(WRITE_AHBM) = 0x0300;
    DMA_SPACE = 0x0670;                 // DSP data to FCRAM

    DMA_CHAN = 0;
    initialized = 1;
}

// returns the staged word holding byte 'address'
static const u16 *fetch_blocks(u32 address, u16 bytes)
{
    u16 lo = (u16)address, hi = (u16)(address >> 16);
    u16 lead = lo & (u16)(BURST_BYTES - 1);
    u16 bursts = (u16)((u16)(lead + bytes + (u16)(BURST_BYTES - 1)) >> 5);
    if (!initialized)
        dma_init();
    wait_ahbm(0xC);
    DMA_CHAN = READ_DMA;
    DMA_CTRL = 0x8000;
    DMA_SRC_LO = (u16)(lo - lead);
    DMA_SRC_HI = hi;
    DMA_DST_LO = (u16)(unsigned)staging;
    DMA_SIZE1 = bursts;
    DMA_CTRL = 0x40C0;
    wait_channel(READ_DMA);
    DMA_CHAN = 0;
    return &staging[lead >> 1];
}

void dma_read(u32 address, u16 words, u16 dst)
{
    while (words) {
        u16 n = words > CHUNK_BYTES / 2 ? (u16)(CHUNK_BYTES / 2) : words;
        copy_words(fetch_blocks(address, (u16)(n << 1)), (s16 *)(unsigned)dst, n);
        address += (u32)n << 1;
        dst += n;
        words -= n;
    }
}

void dma_write(u16 src, u16 words, u32 address)
{
    if (!words)
        return;
    if (!initialized)
        dma_init();
    wait_ahbm(0xC);
    DMA_CHAN = WRITE_DMA;
    DMA_CTRL = 0x8000;
    DMA_SRC_LO = src;
    DMA_SRC_HI = 0;
    DMA_DST_LO = (u16)address;
    DMA_DST_HI = (u16)(address >> 16);
    DMA_SIZE0 = words;
    DMA_SIZE1 = 0;
    DMA_CTRL = 0x40C0;
    wait_channel(WRITE_DMA);
    DMA_CHAN = 0;
}

const u16 *dma_fetch_words(u32 address, u16 words)
{
    return fetch_blocks(address, (u16)(words << 1));
}

void fetch_samples(const fetch_job_t *job)
{
    u16 count = job->stereo ? (u16)(job->count << 1) : job->count;
    s16 *dst = job->dst;
    if (job->format == FMT_PCM16) {
        dma_read(job->address, count, (u16)(unsigned)dst);
        return;
    }
    {
        // PCM8: odd addresses start in a high byte
        const u16 *w = fetch_blocks(job->address, (u16)(count + 1));
        u16 k = 0, pairs;
        if (job->address & 1) {
            dst[0] = (s16)(w[0] & 0xFF00);
            w++;
            k = 1;
        }
        pairs = (u16)((count - k) >> 1);
        pcm8_expand(w, dst + k, pairs);
        k += (u16)(pairs << 1);
        if (k < count)
            dst[k] = (s16)(u16)(w[pairs] << 8);
    }
}
