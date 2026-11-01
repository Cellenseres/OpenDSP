// Teak kernels, block argument in a0.
#ifndef TEAK_KERNELS_H
#define TEAK_KERNELS_H
#include "opus_types.h"
#define TK_PTR(p) ((opus_uint16)(unsigned)(p))
void opus_k_merge(opus_uint16 *block);
void opus_k_haar(opus_uint16 *block);
void opus_k_normres(opus_uint16 *block);
void opus_k_exprot(opus_uint16 *block);
void opus_k_cwrsi(opus_uint16 *block);
void opus_k_scale(opus_int16 n, const opus_int32 *x, opus_int32 *out, opus_int16 count);
void opus_k_sat(opus_int32 *x, opus_int16 count);
void opus_k_ilv(const opus_int32 *src, opus_int32 *dst, opus_int16 count, opus_int16 stride);
void opus_k_dilv(const opus_int32 *src, opus_int32 *dst, opus_int16 count, opus_int16 stride);
opus_int16 opus_k_b2p(const unsigned char *cache, opus_int16 bits);
void opus_k_unq_norm(opus_uint16 *block);
const void *opus_pvq_row(void);
opus_int32 opus_k_alloc_interp(const void **args);
opus_int32 opus_k_alloc_trial(const void **args);
#endif
