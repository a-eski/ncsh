#ifndef ncsh_args_h
#define ncsh_args_h

#include <stdint.h>
#include <stdbool.h>

#define ncsh_TOKENS 10
#define ncsh_TOKEN_BUFFER_SIZE 64

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
	uint_fast8_t count;
	uint_fast8_t max_line_length;
	enum ncsh_Ops* ops;
	char** values;
};

bool ncsh_args_is_valid(struct ncsh_Args args);

struct ncsh_Args ncsh_args_malloc(void);

void ncsh_args_free(struct ncsh_Args args);

void ncsh_args_free_values(struct ncsh_Args args);

#endif // !ncsh_args_h

