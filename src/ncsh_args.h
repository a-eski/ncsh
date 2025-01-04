// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_args_h
#define ncsh_args_h

#include <stddef.h>
#include <stdint.h>

#define ncsh_TOKENS 128

enum ncsh_Ops {
	OP_NONE = 0,
	OP_CONSTANT = 1,
	OP_PIPE = 2,					// |
	OP_STDOUT_REDIRECTION = 3,			// >
	OP_STDOUT_REDIRECTION_APPEND = 4,		// >>
	OP_STDIN_REDIRECTION = 5,			// <
	OP_STDIN_REDIRECTION_APPEND = 6,		// <<
	OP_STDERR_REDIRECTION = 7,			// 2>
	OP_STDERR_REDIRECTION_APPEND = 8,		// 2>>
	OP_STDOUT_AND_STDERR_REDIRECTION = 9,		// &>
	OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10,	// &>>
	OP_BACKGROUND_JOB = 11,				// &
	OP_AND = 12,					// &&
	OP_OR = 13,					// ||
};

struct ncsh_Args {
	uint_fast32_t count;
	size_t max_line_length;
	uint_fast8_t* ops;
	char** values;
};

#endif // !ncsh_args_h

