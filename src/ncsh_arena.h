/* Copyright ncsh by Alex Eski 2025 */

#pragma once

#include <stdint.h>
#include <sys/cdefs.h>

struct ncsh_Arena {
    char* start;
    char* end;
};

#define arena_malloc(arena, count, type) \
    (type*)ncsh_arena_malloc_internal(arena, count, sizeof(type), _Alignof(type))

void* ncsh_arena_malloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment) __attribute_malloc__;

#define arena_realloc(arena, count, type, ptr, old_count) \
    (type*)ncsh_arena_realloc_internal(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* ncsh_arena_realloc_internal(struct ncsh_Arena* arena,
                     uintptr_t count,
                     uintptr_t size,
                     uintptr_t alignment,
                     void* old_ptr,
                     uintptr_t old_count) __attribute_malloc__;
