#ifndef ncsh_args_h
#define ncsh_args_h

#include <stdint.h>
#include <stdbool.h>

#define ncsh_TOKENS 10
#define ncsh_TOKEN_BUFFER_SIZE 64

enum OpCode {
	OP_NONE = 0,
	OP_PIPE,
	OP_INPUT_REDIRECTION,
	OP_INPUT_REDIRECTION_APPEND,
	OP_OUTPUT_REDIRECTION,
	OP_OUTPUT_REDIRECTION_APPEND,
	OP_BACKGROUND_JOB
};

struct ncsh_Args {
	uint_fast8_t count;
	uint_fast8_t max_line_length;
	bool op_code_found;
	uint_fast8_t op_codes[ncsh_TOKENS];
	char** values;
};

bool ncsh_args_is_valid(struct ncsh_Args args);

void ncsh_args_free(struct ncsh_Args args);

#endif // !ncsh_args_h


