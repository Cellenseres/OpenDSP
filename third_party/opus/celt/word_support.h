/* SPDX-License-Identifier: BSD-2-Clause */
#ifndef OPUS_WORD_SUPPORT_H
#define OPUS_WORD_SUPPORT_H

#define OVERRIDE_OPUS_COPY
#define OPUS_COPY(destination, source, count) do { \
   __typeof__(&(destination)[0]) opus_copy_target = (destination); \
   __typeof__(&(source)[0]) opus_copy_source = (source); \
   size_t opus_copy_count = (count); \
   size_t opus_copy_index; \
   for (opus_copy_index = 0; opus_copy_index < opus_copy_count; ++opus_copy_index) \
      opus_copy_target[opus_copy_index] = opus_copy_source[opus_copy_index]; \
} while (0)

#define OVERRIDE_OPUS_MOVE
#define OPUS_MOVE(destination, source, count) do { \
   __typeof__(&(destination)[0]) opus_move_target = (destination); \
   __typeof__(&(source)[0]) opus_move_source = (source); \
   size_t opus_move_count = (count); \
   size_t opus_move_index; \
   if ((uintptr_t)opus_move_target < (uintptr_t)opus_move_source) { \
      for (opus_move_index = 0; opus_move_index < opus_move_count; ++opus_move_index) \
         opus_move_target[opus_move_index] = opus_move_source[opus_move_index]; \
   } else { \
      for (opus_move_index = opus_move_count; opus_move_index > 0; --opus_move_index) \
         opus_move_target[opus_move_index - 1] = opus_move_source[opus_move_index - 1]; \
   } \
} while (0)

#define OVERRIDE_OPUS_CLEAR
#define OPUS_CLEAR(destination, count) do { \
   __typeof__(&(destination)[0]) opus_clear_target = (destination); \
   size_t opus_clear_count = (count); \
   size_t opus_clear_index; \
   for (opus_clear_index = 0; opus_clear_index < opus_clear_count; ++opus_clear_index) \
      opus_clear_target[opus_clear_index] = (__typeof__(*opus_clear_target)){0}; \
} while (0)

#endif
