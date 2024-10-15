/* Copyright ncsh by Alex Eski 2024 */

#include <stdint.h>
#include <stdio.h>

#include "ncsh_debug.h"

void ncsh_debug_line(char* line, uint_fast32_t length) {
	printf("line: %s\n", line);
	printf("length: %lu\n", length);
	fflush(stdout);
}

void ncsh_debug_args(struct ncsh_Args args) {
	printf("args.count: %hhu\n", args.count);
	printf("args.max_line_length: %hhu\n", args.max_line_length);

	for (uint_fast32_t i = 0; i < args.count; i++) {
		printf("args.values[%lu] %s\n", i, args.values[i]);
		printf("args.ops[%lu] %d\n", i, args.ops[i]);
	}
}

