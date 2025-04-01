#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_arena.h"

__attribute_malloc__
void* ncsh_arena_malloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment)
{
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        puts(RED "ncsh: ran out of allocated memory." RESET);
        abort();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    return memset(val, 0, count * size);
}


__attribute_malloc__
void* ncsh_arena_realloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment,
                     void* old_ptr,
                     uintptr_t old_count)
{
    uintptr_t padding = -(uintptr_t)arena->start & (alignment - 1);
    uintptr_t available = (uintptr_t)arena->end - (uintptr_t)arena->start - padding;
    assert(count < available / size);
    if (available == 0 || count > available / size) {
        puts(RED "ncsh: ran out of allocated memory." RESET);
        abort();
    }
    void* val = arena->start + padding;
    arena->start += padding + count * size;
    memset(val, 0, count * size);
    assert(old_ptr);
    return memcpy(val, old_ptr, old_count * size);
}
