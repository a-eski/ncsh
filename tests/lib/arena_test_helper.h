#pragma once

#include <stdlib.h> // used in macros

#define ARENA_TEST_SETUP                                                                                               \
    constexpr int arena_capacity = 1 << 26;                                                                            \
    char* memory = malloc(arena_capacity);                                                                             \
    Arena arena = {.start = memory, .end = memory + (arena_capacity)};                                                 \
    [[maybe_unused]] Arena a = arena

#define ARENA_TEST_TEARDOWN free(memory)

#define SCRATCH_ARENA_TEST_SETUP                                                                                       \
    constexpr int scratch_arena_capacity = 1 << 22;                                                                    \
    char* scratch_memory = malloc(scratch_arena_capacity);                                                             \
    Arena scratch_arena = {.start = scratch_memory, .end = scratch_memory + (scratch_arena_capacity)};                 \
    [[maybe_unused]] Arena s = scratch_arena

#define SCRATCH_ARENA_TEST_TEARDOWN free(scratch_memory)
