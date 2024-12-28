// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_args_h
#define ncsh_args_h

#include <stddef.h>
#include <stdint.h>

#define ncsh_TOKENS 128
#define ncsh_TOKEN_BUFFER_SIZE 256

enum ncsh_Ops {
	OP_NONE = 0,
	OP_CONSTANT = 1,
	OP_PIPE = 2,
	OP_OUTPUT_REDIRECTION = 3,
	OP_OUTPUT_REDIRECTION_APPEND = 4,
	OP_INPUT_REDIRECTION = 5,
	OP_INPUT_REDIRECTION_APPEND = 6,
	OP_BACKGROUND_JOB = 7
};

struct ncsh_Args {
	uint_fast32_t count;
	size_t max_line_length;
	uint_fast8_t* ops;
	char** values;
};

#endif // !ncsh_args_h

