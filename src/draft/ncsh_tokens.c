// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ncsh_tokens.h"
#include "eskilib/eskilib_result.h"

bool ncsh_tokens_is_valid(const struct ncsh_Tokens* args) {
	if (args->count == 0 || args->max_line_length == 0)
		return false;
	else if (args->values == NULL)
		return false;
	else if (args->values[0] == NULL)
		return false;
	else if (args->ops == NULL)
		return false;
	else
		return true;
}

enum eskilib_Result ncsh_tokens_malloc(struct ncsh_Tokens* args) {
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

void ncsh_tokens_free(struct ncsh_Tokens* args) {
	free(args->values);
	args->values = NULL;
	free(args->ops);
	args->ops = NULL;
	args->count = 0;
}

void ncsh_tokens_free_values(struct ncsh_Tokens* args) {
	for (uint_fast32_t i = 0; i < args->count; ++i) {
		if (args->values[i] != NULL) {
			free(args->values[i]);
			args->values[i] = NULL;
		}
	}
	args->count = 0;
}

