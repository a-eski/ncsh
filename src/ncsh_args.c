// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ncsh_args.h"
#include "eskilib/eskilib_result.h"

enum eskilib_Result ncsh_args_malloc(struct ncsh_Args* args) {
	if (args == NULL)
		return E_FAILURE_NULL_REFERENCE;

	args->count = 0;
	args->max_line_length = 0;

	args->values = calloc(ncsh_TOKENS, sizeof(char*));
	if (args->values == NULL)
		return E_FAILURE_MALLOC;

	args->ops = calloc(ncsh_TOKENS, sizeof(uint_fast8_t));
	if (args->ops == NULL) {
		free(args->values);
		return E_FAILURE_MALLOC;
	}

	return E_SUCCESS;
}

void ncsh_args_free(struct ncsh_Args* args) {
	free(args->values);
	args->values = NULL;
	free(args->ops);
	args->ops = NULL;
	args->count = 0;
}

void ncsh_args_free_values(struct ncsh_Args* args) {
	for (uint_fast32_t i = 0; i < args->count; ++i) {
		if (args->values[i] != NULL) {
			free(args->values[i]);
			args->values[i] = NULL;
		}
	}
	args->count = 0;
}

