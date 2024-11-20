// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "ncsh_debug.h"

void ncsh_debug_line(char* buffer, uint_fast32_t buf_position, uint_fast32_t max_buf_position) {
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

void ncsh_debug_config(struct eskilib_String config_location) {
	printf("config value: %s\n", config_location.value);
	printf("config length: %lu\n", config_location.length);
}

