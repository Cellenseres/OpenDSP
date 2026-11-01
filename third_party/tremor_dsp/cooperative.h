/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef TREMOR_COOPERATIVE_H
#define TREMOR_COOPERATIVE_H
#ifdef TREMOR_COOPERATIVE
void tremor_decode_yield(void);
#define TREMOR_DECODE_YIELD() tremor_decode_yield()
#else
#define TREMOR_DECODE_YIELD() ((void)0)
#endif
#endif
