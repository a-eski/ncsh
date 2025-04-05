#pragma once

#define NCSH_ARENA_TEST_SETUP \
  constexpr int arena_capacity = 1<<20; \
  char* memory = calloc(arena_capacity, sizeof(char)); \
  struct ncsh_Arena arena = { .start = memory, .end = memory + (arena_capacity) }

#define NCSH_ARENA_TEST_TEARDOWN \
  free(memory)

#define NCSH_SCRATCH_ARENA_TEST_SETUP \
  constexpr int scratch_arena_capacity = 1<<20; \
  char* scratch_memory = calloc(scratch_arena_capacity, sizeof(char)); \
  struct ncsh_Arena scratch_arena = { .start = scratch_memory, .end = scratch_memory + (scratch_arena_capacity) }

#define NCSH_SCRATCH_ARENA_TEST_TEARDOWN \
  free(scratch_memory)
