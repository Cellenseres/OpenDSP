// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "voice.h"

#define VOICE_STATE_WORDS 68
u16 voice_save(const voice_t *v, vu16 *state);
void voice_restore(voice_t *v, const vu16 *state);
