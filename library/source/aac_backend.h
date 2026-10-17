// SPDX-License-Identifier: BSD-2-Clause
#pragma once
#include "opendsp/aac.h"
typedef struct aac_dsp_backend aac_dsp_backend;
Result aac_backend_open(aac_dsp_backend **out);
Result aac_backend_decode(aac_dsp_backend *d, const void *input, size_t bytes, size_t *used,
                         s16 *pcm, size_t capacity, size_t *frames, opendspAacInfo *info);
void aac_backend_reset(aac_dsp_backend *d);
void aac_backend_close(aac_dsp_backend *d);
Result aac_backend_stats(aac_dsp_backend *d, opendspAacStats *stats);
void opendsp_aac_setup(void);
void opendsp_aac_release(void);
