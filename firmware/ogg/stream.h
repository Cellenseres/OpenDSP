// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

u16 ogg_stream_command(u16 command, vu16 *args, vu16 *reply, u16 busy);
u16 ogg_stream_busy(void);
void ogg_stream_poll(void);
void ogg_stream_cancel(void);
