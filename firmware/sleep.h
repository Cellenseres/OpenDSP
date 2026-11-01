// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "shared_mem.h"

#define SLEEP_WORDS 0x840
#define SLEEP_HEADER_WORDS 24
#define SLEEP_SOURCE_WORDS 81
#define SLEEP_MAGIC 0x5350
#define SLEEP_VERSION 1
#define SLEEP_HEADER ((vu16 *)SHM_DSP_STATUS)
#define SLEEP_EXT_BYTES 0x24000UL
#define SLEEP_RESTORED 58
#define SLEEP_CORE_RESTORED 59

u16 sleep_configure(vu16 *args);
void sleep_save(void);
u16 sleep_restore(void);

typedef char sleep_snapshot_fits[(SLEEP_HEADER_WORDS + 24 * (SLEEP_SOURCE_WORDS + 6) == SLEEP_WORDS) ? 1 : -1];
