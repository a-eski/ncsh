#ifndef ncsh_args_h
#define ncsh_args_h

#include <stdint.h>
#include <stdbool.h>

#define ncsh_TOKENS 10
#define ncsh_TOKEN_BUFFER_SIZE 64

struct ncsh_Args {
	uint_fast8_t count;
	uint_fast8_t max_line_length;
	char** values;
};

bool ncsh_args_is_valid(struct ncsh_Args args);

void ncsh_args_free(struct ncsh_Args args);

void ncsh_args_free_values(struct ncsh_Args args);

#endif // !ncsh_args_h

