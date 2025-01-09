#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ncsh_arena.h"

struct ncsh_Arena {
	size_t size;
	size_t offset;
	void* ptr;
};

static struct ncsh_Arena* arena;

int_fast8_t ncsh_arena_init(size_t size) {
	arena = malloc(sizeof(struct ncsh_Arena));
	if (!arena)
		return EXIT_FAILURE;
	arena->ptr = malloc(size);
	if (!arena->ptr)
		return EXIT_FAILURE;

	arena->size = size;
	arena->offset = 0;

	return EXIT_SUCCESS;
}

void* ncsh_arena_malloc(size_t size) {
	if (arena->offset + size > arena->size)
		return NULL;
	else if (arena->offset + size < size)
		return NULL;

	#pragma GCC diagnostic push // disable pointer arith warnings for this line
	#pragma GCC diagnostic ignored "-Wpointer-arith"
	void* ptr = arena->ptr + arena->offset;
	#pragma GCC diagnostic pop

	arena->offset += size;
	printf("used %zu bytes of arena\n", arena->offset);
	return ptr;
}

void ncsh_arena_exit() {
	if (arena) {
		if (arena->ptr)
			free(arena->ptr);
		free(arena);
	}
}
