/* Copyright ncsh by Alex Eski 2024 */

#include <stdlib.h>

#include "ncsh_args.h"

bool ncsh_args_is_valid(struct ncsh_Args args) {
	if (args.count == 0)
		return false;
	else if (args.values == NULL)
		return false;
	else if (args.values[0] == NULL)
		return false;
	else if (args.ops == NULL)
		return false;
	else
		return true;
}

struct ncsh_Args ncsh_args_malloc(void) {
	struct ncsh_Args args = { .count = 0 };
	args.values = malloc(sizeof(char*) * ncsh_TOKENS);
	if (args.values == NULL)
		exit(EXIT_FAILURE);
	args.ops = malloc(sizeof(enum ncsh_Ops) * ncsh_TOKENS);
	if (args.ops == NULL)
		exit(EXIT_FAILURE);

	return args;
}

void ncsh_args_free(struct ncsh_Args args) {
	free(args.values);
	args.values = NULL;
	free(args.ops);
	args.ops = NULL;
}

void ncsh_args_free_values(struct ncsh_Args args) {
	for (uint_fast8_t i = 0; i < args.count; i++) {
		free(args.values[i]);
		args.values[i] = NULL;
	}
}

