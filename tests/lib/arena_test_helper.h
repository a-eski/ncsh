#pragma once

#include <stdlib.h>

#define ARENA_TEST_SETUP                                                                                               \
    constexpr int arena_capacity = 1 << 20;                                                                            \
    char* memory = malloc(arena_capacity);                                                                             \
    struct Arena arena = {.start = memory, .end = memory + (arena_capacity)}

#define ARENA_TEST_TEARDOWN free(memory)

#define SCRATCH_ARENA_TEST_SETUP                                                                                       \
    constexpr int scratch_arena_capacity = 1 << 20;                                                                    \
    char* scratch_memory = malloc(scratch_arena_capacity);                                                             \
    struct Arena scratch_arena = {.start = scratch_memory, .end = scratch_memory + (scratch_arena_capacity)}

#define SCRATCH_ARENA_TEST_TEARDOWN free(scratch_memory)
