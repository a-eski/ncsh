// Copyright (c) ncsh by Alex Eski 2025

// Still under dev.

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ncsh_arena.h"

struct ncsh_Arena
{
    size_t size;
    size_t offset;
    void *ptr;
};

static struct ncsh_Arena *arena;

int_fast8_t ncsh_arena_init(size_t size)
{
    if (!size || size % 2 != 0)
        return 0;

    arena = malloc(sizeof(struct ncsh_Arena));
    if (!arena)
        return 0;
    arena->ptr = calloc(sizeof(ptrdiff_t), size);
    if (!arena->ptr)
        return 0;

    arena->size = size;
    arena->offset = 0;

    return 1;
}

void *ncsh_arena_malloc(size_t size)
{
    if ((ptrdiff_t)size <= 0)
    {
        puts("ncsh arena: Invalid arena size");
        return NULL;
    }
    else if (arena->offset + size > arena->size)
    {
        puts("ncsh arena: Ran out of room");
        return NULL;
    }
    else if (arena->offset + size < size)
    {
        puts("ncsh arena: Overflow protection");
        return NULL;
    }

#pragma GCC diagnostic push // disable pointer arith warnings for this line
#pragma GCC diagnostic ignored "-Wpointer-arith"
    void *ptr = arena->ptr + arena->offset;
#pragma GCC diagnostic pop

    arena->offset += size;
    printf("used %zu bytes of arena\n", arena->offset);
    return ptr;
}

void ncsh_arena_exit()
{
    if (arena)
    {
        if (arena->ptr)
            free(arena->ptr);
        free(arena);
    }
}
