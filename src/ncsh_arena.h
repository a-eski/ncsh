/* Copyright (c) ncsh by Alex Eski 2025 */

#include <stdint.h>
#ifndef NCSH_ARENA_H_

struct ncsh_Arena {
    char* start;
    char* end;
};

#define alloc(arena, count, type) \
    (type*)ncsh_arena_alloc_internal(arena, count, sizeof(type), _Alignof(type))

void* ncsh_arena_alloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment);

#define arena_realloc(arena, count, type, ptr, old_count) \
    (type*)ncsh_arena_realloc_internal(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* ncsh_arena_realloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment,
                     void* old_ptr,
                     uintptr_t old_count);

#define NCSH_ARENA_H_
#endif /* !NCSH_ARENA_H_ */
