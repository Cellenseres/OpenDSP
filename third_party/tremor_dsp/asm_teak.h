/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef TREMOR_ASM_TEAK_H
#define TREMOR_ASM_TEAK_H

ogg_int32_t tremor_multiply_high(ogg_int32_t, ogg_int32_t);
ogg_int32_t tremor_multiply_shift15(ogg_int32_t, ogg_int32_t);
void tremor_floor_line(ogg_int32_t *, const ogg_int32_t *, const short *);
ogg_uint32_t tremor_divide(ogg_uint32_t, ogg_uint32_t);
void tremor_floor1_unwrap(ogg_int32_t *, short *);
void tremor_floor1_render(const short *, short *);
ogg_uint32_t *tremor_make_word(ogg_uint32_t *, ogg_uint32_t *, unsigned short *);
unsigned short tremor_lengths_run(unsigned short *);
ogg_uint32_t tremor_decpack(unsigned short *);

/* Macro: a helper function may land in another bank. */
#define TREMOR_UDIV 1
#define tremor_udiv(n, d)     ((ogg_uint32_t)(d) - 1u < 0x7fffu && (ogg_uint32_t)(n) < ((ogg_uint32_t)(d) << 15) ?      tremor_divide((n), (d)) : (ogg_uint32_t)(n) / (ogg_uint32_t)(d))

static inline ogg_int32_t MULT32(ogg_int32_t left, ogg_int32_t right)
{
    return tremor_multiply_high(left, right);
}

static inline ogg_int32_t MULT31(ogg_int32_t left, ogg_int32_t right)
{
    return (ogg_uint32_t)tremor_multiply_high(left, right) << 1;
}

static inline ogg_int32_t MULT31_SHIFT15(ogg_int32_t left, ogg_int32_t right)
{
    return tremor_multiply_shift15(left, right);
}

#ifdef TREMOR_TEAK_CROSS
void tremor_cross_product(ogg_int32_t, ogg_int32_t, ogg_int32_t, ogg_int32_t,
                          ogg_int32_t *, ogg_int32_t *);
void tremor_cross_product_negative(ogg_int32_t, ogg_int32_t, ogg_int32_t, ogg_int32_t,
                                   ogg_int32_t *, ogg_int32_t *);
#define XPROD31 tremor_cross_product
#define XNPROD31 tremor_cross_product_negative
#endif
#endif
