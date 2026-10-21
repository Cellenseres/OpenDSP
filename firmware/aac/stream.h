// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"
void aac_stream_init(void);
void aac_stream_poll(void);
void aac_stream_kick(void);
void aac_stream_played(u32 address);
void aac_stream_frame(void);
u16 aac_stream_command(u16 command, vu16 *args, vu16 *reply);
void aac_task_step(void);
void aac_task_main(void);
