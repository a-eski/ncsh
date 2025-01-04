// Copyright (c) ncsh by Alex Eski 2024

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "ncsh_parser.h"
#include "ncsh_args.h"
#include "ncsh_defines.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define STRCMP_2CHAR(in, out) (in[0] == out[0] && in[1] == out[1] ? true : false)

// supported quotes
#define DOUBLE_QUOTE_KEY '\"'
#define SINGLE_QUOTE_KEY '\''
#define BACKTICK_KEY '\`'

// ops
#define PIPE '|'
#define STDIN_REDIRECTION '<'
#define STDOUT_REDIRECTION '>'
#define BACKGROUND_JOB '&'

#define STDIN_REDIRECTION_APPEND_STRING "<<"
#define STDOUT_REDIRECTION_APPEND_STRING ">>"
#define STDERR_REDIRECTION "2>"
#define STDERR_REDIRECTION_APPEND "2>>"
#define STDOUT_AND_STDERR_REDIRECTION "&>"
#define STDOUT_AND_STDERR_REDIRECTION_APPEND "&>>"
#define AND_STRING "&&"
#define OR_STRING "||"

// expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

// currently unsupported
#define BANG '!'

enum eskilib_Result ncsh_args_malloc(struct ncsh_Args* args) {
	if (args == NULL)
		return E_FAILURE_NULL_REFERENCE;

	args->count = 0;
	args->max_line_length = 0;

	args->values = calloc(ncsh_TOKENS, sizeof(char*));
	if (args->values == NULL)
		return E_FAILURE_MALLOC;

	args->ops = calloc(ncsh_TOKENS, sizeof(uint_fast8_t));
	if (args->ops == NULL) {
		free(args->values);
		return E_FAILURE_MALLOC;
	}

	return E_SUCCESS;
}

enum eskilib_Result ncsh_args_malloc_count(int_fast32_t count, struct ncsh_Args* args) {
	if (args == NULL)
		return E_FAILURE_NULL_REFERENCE;

	args->count = count;
	args->max_line_length = 0;

	args->values = calloc(count, sizeof(char*));
	if (args->values == NULL)
		return E_FAILURE_MALLOC;

	args->ops = calloc(count, sizeof(uint_fast8_t));
	if (args->ops == NULL) {
		free(args->values);
		return E_FAILURE_MALLOC;
	}

	return E_SUCCESS;
}

void ncsh_args_free(struct ncsh_Args* args) {
	free(args->values);
	args->values = NULL;
	free(args->ops);
	args->ops = NULL;
	args->count = 0;
}

void ncsh_args_free_values(struct ncsh_Args* args) {
	for (uint_fast32_t i = 0; i < args->count; ++i) {
		if (args->values[i] != NULL) {
			free(args->values[i]);
			args->values[i] = NULL;
		}
	}
	args->count = 0;
}

bool ncsh_is_delimiter(char ch) {
	switch (ch) {
		case ' ':  { return true; }
		// case '\t': { return true; }
		case '\r': { return true; }
		case '\n': { return true; }
		// case '\a': { return true; }
		// case EOF:  { return true; }
		case '\0': { return true; }
		default:   { return false; }
	}
}

enum ncsh_Ops ncsh_op_get_2char(const char* line) {
	if (STRCMP_2CHAR(line, STDIN_REDIRECTION_APPEND_STRING)) {
		return OP_STDIN_REDIRECTION_APPEND;
	}
	else if (STRCMP_2CHAR(line, STDOUT_REDIRECTION_APPEND_STRING)) {
		return OP_STDOUT_REDIRECTION_APPEND;
	}
	else if (STRCMP_2CHAR(line, STDERR_REDIRECTION)) {
		return OP_STDERR_REDIRECTION;
	}
	else if (STRCMP_2CHAR(line, STDERR_REDIRECTION_APPEND)) {
		return OP_STDERR_REDIRECTION_APPEND;
	}
	else if (STRCMP_2CHAR(line, STDOUT_AND_STDERR_REDIRECTION)) {
		return OP_STDOUT_AND_STDERR_REDIRECTION;
	}
	else if (STRCMP_2CHAR(line, STDOUT_AND_STDERR_REDIRECTION_APPEND)) {
		return OP_STDOUT_AND_STDERR_REDIRECTION_APPEND;
	}
	else if (STRCMP_2CHAR(line, AND_STRING)) {
		return OP_AND;
	}
	else if (STRCMP_2CHAR(line, OR_STRING)) {
		return OP_OR;
	}
	else {
		return OP_CONSTANT;
	}
}

enum ncsh_Ops ncsh_op_get(const char* line, size_t length) {
	switch (length) {
		case 0 : { return OP_NONE; }
		case 1 : {
			switch (line[0]) {
			  case PIPE: { return OP_PIPE; }
			  case STDOUT_REDIRECTION: { return OP_STDOUT_REDIRECTION; }
			  case STDIN_REDIRECTION: { return OP_STDIN_REDIRECTION; }
			  case BACKGROUND_JOB: { return OP_BACKGROUND_JOB; }
			  default: { return OP_CONSTANT; }
			}
		}
		case 2 : {
			return ncsh_op_get_2char(line);
		}
		default : { return OP_CONSTANT; }
	}
}

void ncsh_parser_parse(const char* line, size_t length, struct ncsh_Args* args) {
	assert(line);
	assert(length > 0);
	if (!line || length == 0 || length > NCSH_MAX_INPUT) {
		args->max_line_length = 0;
		args->count = 0;
		return;
	}

	char buffer[NCSH_MAX_INPUT];
	size_t buf_position = 0;
	uint_fast32_t double_quotes_count = 0;
	bool glob_found = false;

	for (uint_fast32_t line_position = 0; line_position < length + 1; ++line_position) {
		if (line_position == length ||
			line_position == NCSH_MAX_INPUT - 1 ||
			buf_position == NCSH_MAX_INPUT - 1 ||
			args->count == ncsh_TOKENS)
		{
			args->values[args->count] = NULL;
			break;
		}
		else if (ncsh_is_delimiter(line[line_position]) && (double_quotes_count == 0 || double_quotes_count == 2)) {
			buffer[buf_position] = '\0';

			if (glob_found) {
				glob_t glob_buf = {0};
				size_t glob_len;
				glob(buffer, GLOB_DOOFFS, NULL, &glob_buf);

				for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
					glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
					buf_position = glob_len;
					args->values[args->count] = malloc(buf_position);
					memcpy(args->values[args->count], glob_buf.gl_pathv[i], glob_len);
					args->ops[args->count] = OP_CONSTANT;
					++args->count;

					if (args->max_line_length == 0 || buf_position > args->max_line_length)
						args->max_line_length = buf_position;
				}

				globfree(&glob_buf);
				glob_found = false;
			}
			else {
				args->values[args->count] = malloc(buf_position + 1);
				eskilib_string_copy(args->values[args->count], buffer, buf_position + 1);

				args->ops[args->count] = ncsh_op_get(buffer, buf_position);

				++args->count;

				if (args->max_line_length == 0 || buf_position > args->max_line_length)
					args->max_line_length = buf_position;
			}

			buffer[0] = '\0';
			buf_position = 0;
			double_quotes_count = 0;
		}
		else {
			switch (line[line_position]) {
				case DOUBLE_QUOTE_KEY: {
					++double_quotes_count;
					break;
				}
				case TILDE: {
					char* home = getenv("HOME");
					size_t home_length = strlen(home);
					memcpy(buffer + buf_position, home, home_length);
					buf_position += home_length;
					break;
				}
				case GLOB_STAR:
				case GLOB_QUESTION: {
					glob_found = true;
					buffer[buf_position++] = line[line_position];
					break;
				}
				default: {
					buffer[buf_position++] = line[line_position];
					break;
				}
			}
		}
	}
}

void ncsh_parser_parse_noninteractive(int argc, char** argv, struct ncsh_Args* args) {
	(void)argc;
	(void)argv;
	(void)args;
}

