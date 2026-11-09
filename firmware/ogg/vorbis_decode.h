// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "hw.h"

int vorbis_dsp_session_init(u16 channels);
int vorbis_dsp_session_restart(void);
int vorbis_dsp_packet_decode(vu16 *command, s16 *pcm);
u16 vorbis_dsp_session_status(void);
