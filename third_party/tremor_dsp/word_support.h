/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef TREMOR_WORD_SUPPORT_H
#define TREMOR_WORD_SUPPORT_H
#include <stddef.h>

#ifdef TREMOR_PROGRAM_WINDOWS
#define TREMOR_WINDOW_T LOOKUP_T __attribute__((address_space(1)))
#define TREMOR_WINDOW_SECTION __attribute__((section(".teak_program_data")))
#else
#define TREMOR_WINDOW_T LOOKUP_T
#define TREMOR_WINDOW_SECTION
#endif

#ifdef TREMOR_WORD_STORAGE
#ifdef _LOW_ACCURACY_
#error Low accuracy is unsupported
#endif
#ifdef LIMIT_TO_64kHz
#error Window size restrictions are unsupported
#endif
typedef ogg_int16_t tremor_int8_t;
typedef ogg_uint16_t tremor_uint8_t;
void *tremor_header_scratch(void);
#ifdef __teak__
#undef _ogg_malloc
#undef _ogg_calloc
#undef _ogg_realloc
#undef _ogg_free
#define _ogg_malloc tremor_malloc
#define _ogg_calloc tremor_calloc
#define _ogg_realloc tremor_realloc
#define _ogg_free tremor_free
void *tremor_malloc(size_t);
void *tremor_calloc(size_t, size_t);
void *tremor_realloc(void *, size_t);
void tremor_free(void *);
#endif
#else
typedef char tremor_int8_t;
typedef unsigned char tremor_uint8_t;
#endif

static inline tremor_uint8_t tremor_octet(unsigned long value)
{
    return value & 0xff;
}

static inline size_t tremor_node_storage(unsigned logical_bytes)
{
    return logical_bytes == 1 ? sizeof(tremor_uint8_t) : logical_bytes;
}

static inline size_t tremor_octet_storage(size_t count)
{
#ifdef __teak__
    return count * 2;
#else
    return count;
#endif
}

static inline void tremor_clear_octets(char *destination, size_t count)
{
    volatile unsigned char *output = (volatile unsigned char *)destination;
    size_t index;
    for (index = 0; index < count; ++index) output[index] = 0;
}

static inline int tremor_header_matches(const char *header)
{
    const volatile unsigned char *text = (const volatile unsigned char *)header;
    return text[0] == 'v' && text[1] == 'o' && text[2] == 'r' &&
           text[3] == 'b' && text[4] == 'i' && text[5] == 's';
}
#endif
