// SPDX-License-Identifier: BSD-2-Clause
// Library internals.
#pragma once
#include <3ds.h>

extern bool opendsp_running;

void opendsp_mailbox_setup(void);

// Sends one mailbox request and waits for the answer.
Result opendsp_request(u16 command, const u16 *args, u16 arg_count, u16 *reply, u16 reply_count);

void opendsp_effects_release(void);
