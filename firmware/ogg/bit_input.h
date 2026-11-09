// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "packet.h"

u32 ogg_input_bits(ogg_input *input, long offset, u16 bit, u16 bits);
int ogg_input_window(ogg_input *input, long offset);
