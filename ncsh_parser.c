#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "ncsh_parser.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_args.h"

#define DOUBLE_QUOTE_KEY '\"'

#define PIPE_SYMBOL '|'
#define INPUT_REDIRECTION_SYMBOL '<'
#define INPUT_REDIRECTION_APPEND_SYMBOL "<<"
#define OUTPUT_REDIRECTION_SYMBOL '>'
#define OUTPUT_REDIRECTION_APPEND_SYMBOL ">>"
#define BACKGROUND_JOB_SYMBOL '&'

bool ncsh_is_delimiter(char ch) {
	switch (ch) {
		case ' ': {
			return true;
		}
		case '\t': {
			return true;
		}
		case '\r': {
			return true;
		}
		case '\n': {
			return true;
		}
		case '\a': {
			return true;
		}
		case EOF: {
			return true;
		}
		case '\0': {
			return true;
		}
		default: {
			return false;
		}
	}
}

struct ncsh_Args ncsh_parse(char line[], uint_fast32_t length) {
	char buffer[ncsh_TOKEN_BUFFER_SIZE];
	uint_fast8_t buffer_position = 0;
	uint_fast8_t double_quotes_count = 0;

	struct ncsh_Args args = { .count = 0 };
	args.values = malloc(sizeof(char*) * ncsh_TOKENS);
	if (args.values == NULL) exit(EXIT_FAILURE);

	for (uint_fast8_t line_position = 0; line_position < length + 1; line_position++) {
		if (line_position == length || buffer_position == ncsh_TOKEN_BUFFER_SIZE - 1) {
			args.values[args.count] = NULL;
			break;
		}
		else if (ncsh_is_delimiter(line[line_position]) && (double_quotes_count == 0 || double_quotes_count == 2)) {
			buffer[buffer_position] = '\0';

			args.values[args.count] = malloc(sizeof(char) * (buffer_position + 1));
			eskilib_string_copy(args.values[args.count], buffer, buffer_position + 1);
			args.count++;

			if (args.max_line_length == 0 || buffer_position > args.max_line_length)
				args.max_line_length = buffer_position;

			buffer[0] = '\0';
			buffer_position = 0;
			double_quotes_count = 0;
		}
		else {
			if (line[line_position] == DOUBLE_QUOTE_KEY)
				double_quotes_count++;
			else	
				buffer[buffer_position++] = line[line_position];
		}
	}

	return args;
}

