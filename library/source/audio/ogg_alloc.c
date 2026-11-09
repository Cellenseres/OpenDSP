/* SPDX-License-Identifier: BSD-2-Clause */
#include <stdint.h>
#include <string.h>
#include "codec.h"
#include "../../../third_party/opendsp_alloc.h"

/* Serialize allocation ownership across decoder calls. */
static u32 allocator_lock;
static audio_ogg_memory *owner;
static size_t allocated;
/* Switch Opus scratch under allocator_lock. */
extern char *global_stack;
extern char *scratch_ptr;
#define STREAM_LIMIT (8u * 1024u * 1024u)
#define TOTAL_LIMIT (16u * 1024u * 1024u)
typedef union allocation_header allocation_header;
union allocation_header {
    struct {
        size_t size;
        audio_ogg_memory *owner;
        allocation_header *next, *previous;
        bool scratch;
    } allocation;
    long double alignment;
    void *pointer;
};

void opendsp_audio_ogg_enter(audio_ogg_memory *memory)
{
    while (__atomic_exchange_n(&allocator_lock, 1, __ATOMIC_ACQUIRE))
        svcSleepThread(1000000);
    owner = memory;
    global_stack = owner->opus_stack;
    scratch_ptr = owner->opus_scratch;
}

void opendsp_audio_ogg_leave(void)
{
    allocation_header *h = owner->allocations;
    while (h) {
        allocation_header *next = h->allocation.next;
        if (h->allocation.scratch)
            opendsp_codec_free(h + 1);
        h = next;
    }
    owner->opus_stack = global_stack;
    owner->opus_scratch = scratch_ptr;
    global_stack = scratch_ptr = NULL;
    owner = NULL;
    __atomic_store_n(&allocator_lock, 0, __ATOMIC_RELEASE);
}

void opendsp_audio_ogg_release(void)
{
    while (owner->allocations)
        opendsp_codec_free((allocation_header *)owner->allocations + 1);
    global_stack = scratch_ptr = NULL;
}

static void allocation_failed(void)
{
    if (!owner)
        return;
    opendsp_audio_ogg_release();
    owner->failed = true;
    global_stack = scratch_ptr = NULL;
    longjmp(owner->escape, 1);
}

void *opendsp_codec_malloc(size_t size)
{
    if (!owner || size > SIZE_MAX - sizeof(allocation_header))
        { allocation_failed(); return NULL; }
    size_t bytes = sizeof(allocation_header) + size;
    if (bytes > STREAM_LIMIT - owner->current || bytes > TOTAL_LIMIT - allocated)
        { allocation_failed(); return NULL; }
    allocation_header *h = malloc(bytes);
    if (!h) { allocation_failed(); return NULL; }
    h->allocation.size = size;
    h->allocation.owner = owner;
    h->allocation.previous = NULL;
    h->allocation.next = owner->allocations;
    h->allocation.scratch = false;
    if (h->allocation.next)
        h->allocation.next->allocation.previous = h;
    owner->allocations = h;
    allocated += bytes;
    owner->current += bytes;
    if (owner->peak < owner->current)
        owner->peak = owner->current;
    return h + 1;
}

void opendsp_codec_free(void *ptr)
{
    if (!ptr)
        return;
    allocation_header *h = (allocation_header *)ptr - 1;
    if (h->allocation.previous)
        h->allocation.previous->allocation.next = h->allocation.next;
    else
        h->allocation.owner->allocations = h->allocation.next;
    if (h->allocation.next)
        h->allocation.next->allocation.previous = h->allocation.previous;
    h->allocation.owner->current -= h->allocation.size + sizeof(*h);
    allocated -= h->allocation.size + sizeof(*h);
    free(h);
}

void *opendsp_codec_calloc(size_t count, size_t size)
{
    if (size && count > SIZE_MAX / size)
        { allocation_failed(); return NULL; }
    size_t bytes = count * size;
    void *ptr = opendsp_codec_malloc(bytes);
    if (ptr)
        memset(ptr, 0, bytes);
    return ptr;
}

void *opendsp_codec_realloc(void *ptr, size_t size)
{
    if (!ptr)
        return opendsp_codec_malloc(size);
    if (!size) {
        opendsp_codec_free(ptr);
        return NULL;
    }
    allocation_header *h = (allocation_header *)ptr - 1;
    size_t old = h->allocation.size;
    if (size > old && (size - old > STREAM_LIMIT - owner->current ||
                       size - old > TOTAL_LIMIT - allocated))
        { allocation_failed(); return NULL; }
    if (size > SIZE_MAX - sizeof(*h))
        { allocation_failed(); return NULL; }
    h = realloc(h, sizeof(*h) + size);
    if (!h) { allocation_failed(); return NULL; }
    if (h->allocation.previous)
        h->allocation.previous->allocation.next = h;
    else
        owner->allocations = h;
    if (h->allocation.next)
        h->allocation.next->allocation.previous = h;
    h->allocation.size = size;
    allocated = allocated - old + size;
    owner->current = owner->current - old + size;
    if (owner->peak < owner->current)
        owner->peak = owner->current;
    return h + 1;
}

void *opendsp_codec_scratch(size_t size)
{
    void *ptr = opendsp_codec_malloc(size);
    if (ptr)
        ((allocation_header *)ptr - 1)->allocation.scratch = true;
    return ptr;
}
