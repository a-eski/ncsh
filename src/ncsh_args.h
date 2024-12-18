// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_args_h
#define ncsh_args_h

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
	OP_OR = 9,
	OP_GLOB = 10
};

struct ncsh_Args {
	uint_fast32_t count;
	uint_fast32_t max_line_length;
	uint_fast8_t* ops;
	char** values;
};

bool ncsh_args_is_valid(const struct ncsh_Args* args);

enum eskilib_Result ncsh_args_malloc(struct ncsh_Args* args);

void ncsh_args_free(struct ncsh_Args* args);

void ncsh_args_free_values(struct ncsh_Args* args);

#endif // !ncsh_args_h

