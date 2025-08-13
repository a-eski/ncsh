/* Copyright ncsh (C) by Alex Eski 2025 */
/* arena.h: a simple bump allocator for managing memory */
/* Credit to skeeto and his blogs */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include "arena.h"

void arena_abort__()
{
    puts("ncsh: ran out of allocated memory.");
    // TODO: implement different OOM stragety other than aborting.
    fprintf(stderr, "\nncsh: out of memory, aborting.\n");
    abort();
}

static void (*arena_abort_fn__)() = arena_abort__;

void arena_abort_set(void (*abort_func)())
{
    arena_abort_fn__= abort_func;
}

[[nodiscard]]
__attribute_malloc__
__attribute_alloc_align__((4))
void* arena_malloc__(Arena* restrict arena, uintptr_t count, uintptr_t size,
                            uintptr_t alignment)
{
    assert(arena && count && size && alignment);
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    // debugf("arena space avaiable %zu\n", available);
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        arena_abort_fn__();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    return memset(val, 0, count * size);
}

[[nodiscard]]
__attribute_malloc__
__attribute_alloc_align__((4))
void* arena_realloc__(Arena* restrict arena, uintptr_t count, uintptr_t size,
                                                  uintptr_t alignment, void* old_ptr, uintptr_t old_count)
{
    assert(arena);
    assert(count);
    assert(size);
    assert(alignment);
    assert(old_ptr);
    assert(old_count);

    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    // debugf("arena space avaiable %zu\n", available);
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        arena_abort_fn__();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    memset(val, 0, count * size);
    assert(old_ptr);
    return memcpy(val, old_ptr, old_count * size);
}
