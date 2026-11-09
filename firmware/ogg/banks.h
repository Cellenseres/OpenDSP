// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

u16 codec_bank_command(u16 command, vu16 *args, vu16 *reply, u16 busy);
int codec_bank_poll(void);
u16 codec_bank_ready(u16 bank);
u16 codec_bank_claim(u16 token, u32 parameters[4]);
void codec_bank_resume(void);
