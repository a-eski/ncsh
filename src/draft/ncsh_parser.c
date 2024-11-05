// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <glob.h>
#include <string.h>

#include "ncsh_parser.h"
#include "../ncsh_args.h"
#include "../eskilib/eskilib_string.h"

#define DOUBLE_QUOTE_KEY '\"'

#define PIPE '|'
#define INPUT_REDIRECTION '<'
#define OUTPUT_REDIRECTION '>'
#define BACKGROUND_JOB '&'

#define INPUT_REDIRECTION_APPEND_STRING "<<"
#define OUTPUT_REDIRECTION_APPEND_STRING ">>"
#define AND "&&"
#define OR "||"

bool ncsh_is_delimiter(char ch) {
	switch (ch) {
		case ' ':  { return true; }
		case '\t': { return true; }
		case '\r': { return true; }
		case '\n': { return true; }
		case '\a': { return true; }
		case EOF:  { return true; }
		case '\0': { return true; }
		default:   { return false; }
	}
}

enum ncsh_Ops ncsh_op_get(char line[], uint_fast32_t length) {
	if (line == NULL || length == 0)
		return OP_NONE;

	if (length == 1) {
		switch (line[0]) {
			case PIPE: {
				return OP_PIPE;
			}
			case OUTPUT_REDIRECTION: {
				return OP_OUTPUT_REDIRECTION;
			}
			case INPUT_REDIRECTION: {
				return OP_INPUT_REDIRECTION;
			}
			case BACKGROUND_JOB: {
				return OP_BACKGROUND_JOB;
			}
			default: {
				return OP_CONSTANT;
			}
		}
	}

	if (eskilib_string_equals(line, OUTPUT_REDIRECTION_APPEND_STRING, length))
		return OP_OUTPUT_REDIRECTION_APPEND;
	else if (eskilib_string_equals(line, INPUT_REDIRECTION_APPEND_STRING, length))
		return OP_INPUT_REDIRECTION_APPEND;

	return OP_CONSTANT;
}

struct ncsh_Args ncsh_parse(char line[], uint_fast32_t length, struct ncsh_Args args) {
	if (line == NULL || args.values == NULL || args.ops == NULL) {
		args.max_line_length = 0;
		args.count = 0;
		return args;
	}

	if (length == 0 || length > ncsh_TOKEN_BUFFER_SIZE) {
		args.max_line_length = 0;
		args.count = 0;
		return args;
	}

	char buffer[ncsh_TOKEN_BUFFER_SIZE];
	uint_fast32_t buffer_position = 0;
	uint_fast32_t double_quotes_count = 0;

	for (uint_fast32_t line_position = 0; line_position < length + 1; ++line_position) {
		if (line_position == length || line_position == ncsh_TOKEN_BUFFER_SIZE - 1 ||
			buffer_position == ncsh_TOKEN_BUFFER_SIZE - 1 || args.count == ncsh_TOKEN_BUFFER_SIZE - 1) {
			args.values[args.count] = NULL;
			break;
		}
		else if (ncsh_is_delimiter(line[line_position]) && (double_quotes_count == 0 || double_quotes_count == 2)) {
			buffer[buffer_position] = '\0';

			args.values[args.count] = malloc(sizeof(char) * (buffer_position + 1));
			eskilib_string_copy(args.values[args.count], buffer, buffer_position + 1);

			args.ops[args.count] = ncsh_op_get(buffer, buffer_position);

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

void ncsh_token_init(struct ncsh_Token* token, size_t length) {
	token->data = malloc(length);
	token->data[0] = '\0';
	token->token_type = TT_NULL;
	token->next = NULL;
}

int ncsh_type_char(char character) {
	switch(character) {
		case '\'':
			return TT_QUOTE;
			break;
		case '\"':
			return TT_DOUBLE_QUOTE;
			break;
		case '|':
			return TT_PIPE;
			break;
		case '&':
			return TT_AMPERSAND;
			break;
		case ' ':
			return TT_WHITESPACE;
			break;
		case ';':
			return TT_SEMICOLON;
			break;
		case '\\':
			return TT_ESCAPE_SEQUENCE;
			break;
		case '\t':
			return TT_TAB;
			break;
		case '\n':
			return TT_NEWLINE;
			break;
		case '>':
			return TT_GREATER;
			break;
		case '<':
			return TT_LESSER;
			break;
		case 0:
			return TT_NULL;
			break;
		default:
			return TT_GENERAL;
	};

	return TT_GENERAL;
}

void ncsh_quotes_strip(char* source, char* destination, size_t source_length) {
	source_length -= 1;
	if (source_length <= 1) {
		eskilib_string_copy(destination, source, source_length);
		return;
	}

	char last_quote = '\0';
	char character;
	size_t j = 0;

	for (size_t i = 0; i < source_length; ++i) {
		character = source[i];
		if ((character == '\'' || character == '\"') && last_quote == '\0')
			last_quote = character;
		else if (character == last_quote)
			last_quote = '\0';
		else
			destination[j++] = character;
	}

	destination[j] = '\0';
}


int_fast32_t ncsh_parse_v2(char line[], size_t length, struct ncsh_Tokens* tokens) {
	assert(line != NULL);
	assert(tokens != NULL);
	if (line == NULL || tokens == NULL)
		return -1;

	if (length == 0) {
		tokens->token_count = 0;
		return 0;
	}

	tokens->tokens_linked_list = malloc(sizeof(struct ncsh_Token));
	struct ncsh_Token* token = tokens->tokens_linked_list;

	int_fast32_t i = 0;
	int_fast32_t j = 0;
	int_fast32_t n = 0;
	char character = '\0';
	int character_type = 0;
	int state = STATE_GENERAL;

	// iterate through and populate initial linked list of tokens
	do {
		character = line[i];
		character_type = ncsh_type_char(character);

		if (state == STATE_GENERAL) {
			switch (character_type) {
				case TT_QUOTE:
					state = STATE_IN_QUOTE;
					token->data[j] = TT_QUOTE;
					++j;
					token->token_type = TT_TOKEN;
					break;

				case TT_DOUBLE_QUOTE:
					state = STATE_IN_DOUBLE_QUOTE;
					token->data[j] = TT_DOUBLE_QUOTE;
					++j;
					token->token_type = TT_TOKEN;
					break;

				case TT_ESCAPE_SEQUENCE:
					state = STATE_IN_ESCAPE_SEQUENCE;
					token->data[j] = line[++i];
					++j;
					token->token_type = TT_TOKEN;
					break;

				case TT_GENERAL:
					token->data[j] = character;
					++j;
					token->token_type = TT_TOKEN;
					break;

				case TT_WHITESPACE:
					if (j > 0) {
						token->data[j] = '\0';
						token->next = malloc(sizeof(struct ncsh_Token));
						token = token->next;
						ncsh_token_init(token, length - i);
						j = 0;
					}
					break;

				case TT_SEMICOLON:
				case TT_GREATER:
				case TT_LESSER:
				case TT_AMPERSAND:
				case TT_PIPE:
					if (j > 0) {
						token->data[j] = '\0';
						token->next = malloc(sizeof(struct ncsh_Token));
						token = token->next;
						ncsh_token_init(token, length - i);
						j = 0;
					}

					token->data[0] = character_type;
					token->data[1] = '\0';
					token->token_type = character_type;

					token->next = malloc(sizeof(struct ncsh_Token));
					token = token->next;
					ncsh_token_init(token, length - i);

					break;
			}
		}
		else if (state == STATE_IN_DOUBLE_QUOTE) {
			token->data[j] = character;
			++j;
			if (character_type == TT_DOUBLE_QUOTE)
				state = STATE_GENERAL;
		}
		else if (state == STATE_IN_QUOTE) {
			token->data[j] = character;
			++j;
			if (character_type == TT_QUOTE)
				state = STATE_GENERAL;
		}

		if (character_type == TT_NULL) {
			if (j > 0) {
				token->data[j] = '\0';
				++n;
				j = 0;
			}
		}

		++i;
	} while (character != '\0');

	// reset token to head so we can expand wildcards and count tokens
	token = tokens->tokens_linked_list;
	int_fast32_t token_count = 0;

	while (token != NULL) {
		if (token->token_type == TT_TOKEN) {
			glob_t glob_buffer;
			glob(token->data, GLOB_TILDE, NULL, &glob_buffer);

			if (glob_buffer.gl_pathc > 0) {
				token_count += glob_buffer.gl_pathc;

				struct ncsh_Token* saved_token_to_reattach = token->next;

				free(token->data);
				int pathv_length = strlen(glob_buffer.gl_pathv[0] + 1);
				token->data = malloc(pathv_length);
				eskilib_string_copy(token->data, glob_buffer.gl_pathv[0], pathv_length);

				for (size_t i = 1; i < glob_buffer.gl_pathc; ++i) {
					token->next = malloc(sizeof(struct ncsh_Token));
					pathv_length = strlen(glob_buffer.gl_pathv[i]);
					ncsh_token_init(token, pathv_length);
					token = token->next;
					token->token_type = TT_TOKEN;
					eskilib_string_copy(token->data, glob_buffer.gl_pathv[i], pathv_length);
				}

				token->next = saved_token_to_reattach;
			}
			else {
				size_t token_data_length = strlen(token->data) + 1;
				char* stripped_data = malloc(token_data_length);
				ncsh_quotes_strip(token->data, stripped_data, token_data_length);
				free(token->data);
				token->data = stripped_data;
				++token_count;
			}
		}

		token = token->next;
	}

	tokens->token_count = token_count;
	return token_count;
}

