/* OpenDSP build configuration, BSD-2-Clause. */
#ifndef OGG_CONFIG_TYPES_H
#define OGG_CONFIG_TYPES_H
/* Match Tremor's int-valued exponents on ILP32 newlib, where int32_t is long. */
typedef short ogg_int16_t;
typedef unsigned short ogg_uint16_t;
typedef int ogg_int32_t;
typedef unsigned int ogg_uint32_t;
typedef long long ogg_int64_t;
typedef unsigned long long ogg_uint64_t;
#endif
