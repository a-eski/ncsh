// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "ncsh_debug.h"

void ncsh_debug_line(char* buffer, size_t buf_position, size_t max_buf_position) {
	printf("buffer: %s\n", buffer);
	printf("buf_position: %lu\n", buf_position);
	printf("max_buf_position %lu\n", max_buf_position);
	fflush(stdout);
}

void ncsh_debug_args(struct ncsh_Args args) {
	printf("args.count: %lu\n", args.count);
	printf("args.max_line_length: %lu\n", args.max_line_length);

	for (uint_fast32_t i = 0; i < args.count; ++i) {
		printf("args.values[%lu] %s\n", i, args.values[i]);
		printf("args.ops[%lu] %d\n", i, args.ops[i]);
	}
}

void ncsh_debug_string(struct eskilib_String string, const char* name) {
	printf("%s value: %s\n", name, string.value);
	printf("%s length: %lu\n", name, string.length);
}

