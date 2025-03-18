#define NCSH_ARENA_TEST_SETUP \
  constexpr int arena_capacity = 1<<16; \
  char* memory = malloc(arena_capacity); \
  struct ncsh_Arena arena = { .start = memory, .end = memory + (arena_capacity) }

#define NCSH_ARENA_TEST_TEARDOWN \
  free(memory)

#define NCSH_SCRATCH_ARENA_TEST_SETUP \
  constexpr int scratch_arena_capacity = 1<<16; \
  char* scratch_memory = malloc(scratch_arena_capacity); \
  struct ncsh_Arena scratch_arena = { .start = scratch_memory, .end = scratch_memory + (scratch_arena_capacity) }

#define NCSH_SCRATCH_ARENA_TEST_TEARDOWN \
  free(scratch_memory)
