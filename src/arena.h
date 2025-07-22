/* Copyright ncsh (C) by Alex Eski 2025 */
/* arena.h: a simple bump allocator for managing memory */
/* Credit to skeeto and his blogs */

#pragma once

#include <stdint.h>
#include <sys/cdefs.h> // for __attribute_malloc__

#include "defines.h"

typedef struct {
    char* start;
    char* end;
} Arena;

#define arena_malloc(arena, count, type) (type*)arena_malloc_internal(arena, count, sizeof(type), _Alignof(type))

void* arena_malloc_internal(Arena* rst arena, uintptr_t count, uintptr_t size,
                            uintptr_t alignment)
    __attribute_malloc__
    __attribute_alloc_align__((4));

#define arena_realloc(arena, count, type, ptr, old_count)                                                              \
    (type*)arena_realloc_internal(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* arena_realloc_internal(Arena* rst arena, uintptr_t count, uintptr_t size, uintptr_t alignment, void* old_ptr,
                             uintptr_t old_count)
    __attribute_malloc__
    __attribute_alloc_align__((4));
