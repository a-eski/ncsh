// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_tokens_h
#define ncsh_tokens_h

#include <stdint.h>

#include "eskilib/eskilib_result.h"

#define ncsh_TOKENS 128
#define ncsh_TOKEN_BUFFER_SIZE 128

enum ncsh_Ops {
	OP_NONE = 0,
	OP_CONSTANT = 1,
	OP_PIPE = 2,
	OP_OUTPUT_REDIRECTION = 3,
	OP_OUTPUT_REDIRECTION_APPEND = 4,
	OP_INPUT_REDIRECTION = 5,
	OP_INPUT_REDIRECTION_APPEND = 6,
	OP_BACKGROUND_JOB = 7,
	OP_AND = 8,
	OP_OR = 9
};

struct ncsh_Tokens {
	uint_fast32_t count;
	uint_fast32_t max_line_length;
	uint_fast8_t* ops;
	char** values;

	bool syntax_error;
	uint_fast32_t output_redirect_found;
	uint_fast32_t input_redirect_found;
	uint_fast32_t number_of_pipe_commands;
	char* output_file;
	char* input_file;
};

bool ncsh_tokens_is_valid(const struct ncsh_Tokens* args);

enum eskilib_Result ncsh_tokens_malloc(struct ncsh_Tokens* args);

void ncsh_tokens_free(struct ncsh_Tokens* args);

void ncsh_tokens_free_values(struct ncsh_Tokens* args);

#endif // !ncsh_tokens_h

