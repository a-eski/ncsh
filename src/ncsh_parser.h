// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_parser_h
#define ncsh_parser_h

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "eskilib/eskilib_result.h"

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
	size_t* lengths;
	uint_fast8_t* ops;
	char** values;
};

bool ncsh_parser_args_is_valid(const struct ncsh_Args* args);

enum eskilib_Result ncsh_parser_args_malloc(struct ncsh_Args* args);

enum eskilib_Result ncsh_parser_args_malloc_count(int_fast32_t count, struct ncsh_Args* args);

void ncsh_parser_args_free(struct ncsh_Args* args);

void ncsh_parser_args_free_values(struct ncsh_Args* args);

void ncsh_parser_parse(const char* line, size_t length, struct ncsh_Args* args);

void ncsh_parser_parse_noninteractive(int argc, char** argv, struct ncsh_Args* args);

#endif // !ncsh_parser_h

