#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ncsh_tokenize.h"
#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

struct ncsh_Tokens ncsh_tokens_malloc(void) {
	struct ncsh_Tokens tokens = {0};
	tokens.ops = malloc(sizeof(enum ncsh_Ops) * ncsh_TOKENS);
	tokens.tokens = malloc(sizeof(char**) * ncsh_TOKENS);
	return tokens;
}

void ncsh_tokens_free(struct ncsh_Tokens tokens) {
	free(tokens.tokens);
	tokens.tokens = NULL;
	free(tokens.ops);
	tokens.ops = NULL;
}

void ncsh_tokens_values_free(struct ncsh_Tokens tokens) {
	for (uint_fast8_t i = 0; tokens.tokens[i] != NULL; i++) {
		for (uint_fast8_t j = 0; tokens.tokens[i][j] != NULL; j++) {
			free(tokens.tokens[i][j]);
		}
		free(tokens.tokens[i]);
		tokens.tokens[i] = NULL;
	}
}

struct ncsh_Tokens ncsh_tokenize(struct ncsh_Args args, struct ncsh_Tokens tokens) {
	uint_fast32_t token_buffer_position = 0;
	uint_fast32_t token_sub_buffer_position = 0;
	uint_fast32_t token_last_malloced = 0;

	for (uint_fast8_t i = 0; i < args.count; i++) {
		if (token_buffer_position >= token_last_malloced) {
			tokens.tokens[token_buffer_position] = malloc(sizeof(char*) * ncsh_TOKENS);
			++token_last_malloced;
		}
		tokens.tokens[token_buffer_position][token_sub_buffer_position] = malloc(sizeof(char) * args.max_line_length + 1);
		eskilib_string_copy(tokens.tokens[token_buffer_position][token_sub_buffer_position],
			args.values[i],
			args.max_line_length + 1);
		tokens.tokens[token_buffer_position][token_sub_buffer_position + 1] = NULL;

		if (args.ops[i] != OP_CONSTANT) {
			tokens.ops[token_buffer_position] = args.ops[i];
			++token_buffer_position;
			token_sub_buffer_position = 0;
		}
		else {
			tokens.ops[token_buffer_position] = OP_CONSTANT;
			++token_sub_buffer_position;
		}
	}

	return tokens;
}

