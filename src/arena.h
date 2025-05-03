/* Copyright ncsh (C) by Alex Eski 2025 */
/* arena.h: a simple bump allocator for managing memory */
/* Credit to skeeto and his blogs */

#pragma once

#include <setjmp.h>
#include <stdint.h>
#include <sys/cdefs.h>

struct Arena {
    char* start;
    char* end;
};

#define arena_malloc(arena, count, type) (type*)arena_malloc_internal(arena, count, sizeof(type), _Alignof(type))

void* arena_malloc_internal(struct Arena* arena, uintptr_t count, uintptr_t size,
                            uintptr_t alignment) __attribute_malloc__;

#define arena_realloc(arena, count, type, ptr, old_count)                                                              \
    (type*)arena_realloc_internal(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* arena_realloc_internal(struct Arena* arena, uintptr_t count, uintptr_t size, uintptr_t alignment, void* old_ptr,
                             uintptr_t old_count) __attribute_malloc__;
