/* Copyright ncsh (C) by Alex Eski 2025 */
/* arena.h: a simple bump allocator for managing memory */
/* Credit to skeeto and his blogs */

#pragma once

#include <stdint.h>
#include <sys/cdefs.h> // for __attribute_malloc__
#include <time.h>

typedef struct {
    char* start;
    char* end;
} Arena;

/* arena_abort_fn_set
 * Set the function to be called if the arena is full and the requested memory can't be allocated in the arena.
 */
void arena_abort_fn_set(void (*abort_func)());

/* arenas_new
 * Wrap in a function which returns a char*.
 * Free the char* which is returned when the arenas are no longer needed.
 */
#define arenas_new(arena, arena_size, scratch, scratch_size, memory)                                \
    constexpr int arena_capacity = arena_size;                                                      \
    constexpr int scratch_capacity = scratch_size;                                                  \
    constexpr int total_capacity = arena_capacity + scratch_capacity;                               \
                                                                                                    \
    memory = malloc(total_capacity);                                                                \
    if (!memory) {                                                                                  \
        return NULL;                                                                                \
    }                                                                                               \
                                                                                                    \
    arena = (Arena){.start = memory, .end = memory + (arena_capacity)};                             \
    char* scratch_memory_start = memory + (arena_capacity + 1);                                     \
    scratch =                                                                                       \
        (Arena){.start = scratch_memory_start, .end = scratch_memory_start + (scratch_capacity)};   \
    return memory;

/* arena_malloc
 * Call to allocate in the arena.
 * Convience wrapper for arena_malloc__
 */
#define arena_malloc(arena, count, type) (type*)arena_malloc__(arena, count, sizeof(type), _Alignof(type))

void* arena_malloc__(Arena* restrict arena, uintptr_t count, uintptr_t size,
                            uintptr_t alignment)
    __attribute_malloc__
    __attribute_alloc_align__((4));

/* arena_realloc
 * Call to reallocate in the arena.
 * Convience wrapper for arena_realloc__
 */
#define arena_realloc(arena, count, type, ptr, old_count)                                                              \
    (type*)arena_realloc__(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* arena_realloc__(Arena* restrict arena, uintptr_t count, uintptr_t size, uintptr_t alignment, void* old_ptr,
                             uintptr_t old_count)
    __attribute_malloc__
    __attribute_alloc_align__((4));
