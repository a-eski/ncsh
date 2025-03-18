/* Copyright (c) ncsh by Alex Eski 2025 */

#include <stdint.h>
#ifndef NCSH_ARENA_H_

#define alloc(arena, count, type) \
    (type*)ncsh_arena_new_internal(arena, count, sizeof(type), _Alignof(type))

struct ncsh_Arena {
    char* start;
    char* end;
};

void* ncsh_arena_new_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment);

#define NCSH_ARENA_H_
#endif /* !NCSH_ARENA_H_ */
