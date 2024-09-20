#include <stdint.h>
#include <stdio.h>

#include "shl_debug.h"
#include "shl_types.h"

void shl_debug_line(char* line, uint_fast32_t length)
{
	printf("line.line: %s\n", line);
	printf("line.length: %lu\n", length);
}

void shl_debug_args(struct shl_Args args)
{
	printf("args.count: %lu\n", args.count);
	printf("args.maxLineSize: %lu\n", args.maxLineSize);
	for (uint_fast32_t i = 0; i < args.count; i++)
		printf("args[%lu] %s\n", i, *(args.lines + i));
}

void shl_debug_launch_process(struct shl_Args args)
{
	printf("args.count: %lu\n", args.count);
	printf("args.maxLineSize: %lu\n", args.maxLineSize);
	printf("args->lines[0]: %s\n", args.lines[0]);
	for (uint_fast32_t i = 1; i < args.count; i++)
		printf("args[%lu] %s\n", i, *(args.lines + i));
}

