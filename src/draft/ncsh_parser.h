// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_parser_h
#define ncsh_parser_h

#include <stddef.h>
#include <stdint.h>

enum ncsh_TokenType {
	TT_GENERAL = -1,
	TT_TOKEN = -1,
	TT_NULL = 0,
	TT_PIPE = '|',
	TT_AMPERSAND = '&',
	TT_QUOTE = '\'',
	TT_DOUBLE_QUOTE = '\"',
	TT_SEMICOLON = ';',
	TT_WHITESPACE = ' ',
	TT_ESCAPE_SEQUENCE = '\\',
	TT_TAB = '\t',
	TT_NEWLINE = '\n',
	TT_GREATER = '>',
	TT_LESSER = '<'
};

enum {
	STATE_IN_DOUBLE_QUOTE,
	STATE_IN_QUOTE,
	STATE_IN_ESCAPE_SEQUENCE,
	STATE_GENERAL
};

struct ncsh_Token;

struct ncsh_Token {
	int token_type;
	size_t data_length;
	char* data;
	struct ncsh_Token* next;
};

struct ncsh_Tokens {
	int_fast32_t token_count;
	struct ncsh_Token* tokens_linked_list;
};

struct ncsh_Args ncsh_parse(char line[], uint_fast32_t length, struct ncsh_Args args);

int_fast32_t ncsh_parse_v2(char line[], size_t length, struct ncsh_Tokens* tokens);

#endif // !ncsh_parser_h

