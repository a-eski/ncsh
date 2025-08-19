/* Copyright ncsh (C) by Alex Eski 2025 */
/* arena.h: a simple bump allocator for managing memory */
/* Credit to skeeto and his blogs for inspiration */

#pragma once

#include <stdint.h>
#include <sys/cdefs.h> // for __attribute_malloc__

#if defined(__GNUC__) || defined(__clang__) || defined(__attribute__malloc__)
#   define ATTR_MALLOC __attribute_malloc__
#elif defined(__malloc_like)
#   define ATTR_MALLOC __malloc_like
#else
#   define ATTR_MALLOC
#endif

#if defined(__GNUC__) || defined(__clang__) || defined(__attribute__alloc_align__)
#   define ATTR_ALLOC_ALIGN(pos) __attribute_alloc_align__((pos))
#elif defined(__alloc_align)
#   define ATTR_ALLOC_ALIGN(pos) __alloc_align((pos))
#else
#   define ATTR_ALLOC_ALIGN(pos)
#endif

typedef struct {
    char* start;
    char* end;
} Arena;

/* arena_abort_fn_set
 * Set the function to be called if the arena is full and the requested memory can't be allocated in the arena.
 * abort_func should call exit, abort, or longjmp.
 */
void arena_abort_fn_set(void (*abort_func)());

/* arena_malloc
 * Call to allocate in the arena.
 * Convience wrapper for arena_malloc__
 */
#define arena_malloc(arena, count, type) (type*)arena_malloc__(arena, count, sizeof(type), _Alignof(type))

void* arena_malloc__(Arena* restrict arena, uintptr_t count, uintptr_t size,
                            uintptr_t alignment)
    ATTR_MALLOC
    ATTR_ALLOC_ALIGN(4);

/* arena_realloc
 * Call to reallocate in the arena.
 * Convience wrapper for arena_realloc__
 */
#define arena_realloc(arena, count, type, ptr, old_count)                                                              \
    (type*)arena_realloc__(arena, count, sizeof(type), _Alignof(type), ptr, old_count);

void* arena_realloc__(Arena* restrict arena, uintptr_t count, uintptr_t size, uintptr_t alignment, void* old_ptr,
                             uintptr_t old_count)
    ATTR_MALLOC
    ATTR_ALLOC_ALIGN(4);
