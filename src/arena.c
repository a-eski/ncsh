#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include "arena.h"
#include "eskilib/ecolors.h"

void arena_abort_internal(struct Arena* arena)
{
    puts(RED "ncsh: ran out of allocated memory." RESET);
    assert(arena->exit);
    // TODO: implement different OOM stragety other than jumping to exit or aborting.
    if (arena->exit) {
        fprintf(stderr, "\n" RED "ncsh: out of memory, jumping to exit." RESET);
        longjmp(*arena->exit, 1);
    }
    else {
        fprintf(stderr, "\n" RED "ncsh: out of memory, aborting." RESET);
        abort();
    }
}

__attribute_malloc__ void* arena_malloc_internal(struct Arena* arena, uintptr_t count, uintptr_t size,
                                                 uintptr_t alignment)
{
    assert(arena && count && size && alignment);
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        arena_abort_internal(arena);
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    return memset(val, 0, count * size);
}

__attribute_malloc__ void* arena_realloc_internal(struct Arena* arena, uintptr_t count, uintptr_t size,
                                                  uintptr_t alignment, void* old_ptr, uintptr_t old_count)
{
    assert(arena && count && size && alignment && old_ptr && old_count);
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        arena_abort_internal(arena);
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    memset(val, 0, count * size);
    assert(old_ptr);
    return memcpy(val, old_ptr, old_count * size);
}
