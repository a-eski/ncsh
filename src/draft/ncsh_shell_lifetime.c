#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct ncsh_Arena {
	size_t size;
	size_t offset;
	void* ptr;
};

int_fast8_t ncsh_shell_lifetime_init(size_t size, struct ncsh_Arena* arena) {
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

void* ncsh_shell_lifetime_malloc(size_t size, struct ncsh_Arena* arena) {
	if (arena->offset + size > arena->size)
		return NULL;

	void* ptr = arena->ptr + arena->offset;
	arena->offset += size;
	printf("used %zu bytes of arena", arena->offset);
	return ptr;
}

void ncsh_shell_lifetime_exit(struct ncsh_Arena* arena) {
	if (arena) {
		if (arena->ptr)
			free(arena->ptr);
		free(arena);
	}
}
