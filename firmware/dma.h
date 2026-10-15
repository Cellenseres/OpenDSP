// SPDX-License-Identifier: BSD-2-Clause
// FCRAM access through AHBM and DMA.
#pragma once
#include "hw.h"

void dma_read(u32 address, u16 words, u16 dst);
void dma_write(u16 src, u16 words, u32 address);
void dma_quiesce(void);
const u16 *dma_fetch_words(u32 address, u16 words);
