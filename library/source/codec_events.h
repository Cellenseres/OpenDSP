/* SPDX-License-Identifier: BSD-2-Clause */
#pragma once
#include <3ds.h>
Result opendsp_codec_events_start(void);
void opendsp_codec_events_stop(void);
bool opendsp_codec_events_active(void);
void opendsp_codec_wait(s64 nanoseconds);
void opendsp_mailbox_wait(s64 nanoseconds);
