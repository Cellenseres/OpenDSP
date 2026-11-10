// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include <3ds.h>

// Core 0 load via idle thread.
// The kernel has no thread times.
void cpu_meter_start(void);
void cpu_meter_stop(void);
bool cpu_meter_idle(u32 *ticks); // running sum; wraps, use differences
