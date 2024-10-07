/* Copyright Alex Eski 2024 */

#include <stdint.h>
#include <stdio.h>

#include "ncsh_debug.h"

void ncsh_debug_line(char* line, uint_fast32_t length) {
	printf("line.line: %s\n", line);
	printf("line.length: %lu\n", length);
	fflush(stdout);
}

void ncsh_debug_args(struct ncsh_Args args) {
	printf("args.count: %hhu\n", args.count);
	printf("args.maxLineSize: %hhu\n", args.max_line_length);

	for (uint_fast32_t i = 0; i < args.count; i++) {
		printf("args.lines[%lu] %s\n", i, args.values[i]);
	}
}

