// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_tokenizer_h
#define ncsh_tokenizer_h

#include <stdint.h>

#include "ncsh_parser.h"

struct ncsh_Tokens {
	uint_fast32_t stdout_redirect_found_index;
	uint_fast32_t stdin_redirect_found_index;
	uint_fast32_t stderr_redirect_found_index;
	uint_fast32_t stdout_and_stderr_redirect_found_index;
	uint_fast32_t number_of_pipe_commands;
	char* output_file;
	char* input_file;
};

int_fast32_t ncsh_tokenizer_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens);

#endif // !ncsh_tokenizer_h

