// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

#define OGG_TASK_CONTEXT ((vu16 *)0x0f80)

void ogg_task_step(void);
void ogg_task_yield(void);
void ogg_task_main(void);
u16 ogg_task_read(u16 address);
void ogg_task_write(u16 address, u16 value);
