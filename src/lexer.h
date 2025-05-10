#pragma once

#include <stddef.h>
#include <stdint.h>

#include "arena.h"
#include "defines.h"

// supported quotes
#define SINGLE_QUOTE_KEY '\''
#define DOUBLE_QUOTE_KEY '\"'
#define SINGLE_QUOTE_KEY '\''
#define BACKTICK_QUOTE_KEY '`'
#define OPENING_PARAN '('
#define CLOSING_PARAN ')'

// ops
#define PIPE '|'
#define STDIN_REDIRECTION '<'
#define STDOUT_REDIRECTION '>'
#define BACKGROUND_JOB '&'

#define STDOUT_REDIRECTION_APPEND ">>"
#define STDERR_REDIRECTION "2>"
#define STDERR_REDIRECTION_APPEND "2>>"
#define STDOUT_AND_STDERR_REDIRECTION "&>"
#define STDOUT_AND_STDERR_REDIRECTION_APPEND "&>>"
#define AND "&&"
#define OR "||"

// ops: multiple
#define DOLLAR_SIGN '$'
#define VARIABLE '$'
#define MATH_EXPRESSION_START "$("
#define MATH_EXPRESSION_END ')'

// ops: numeric
#define ADD '+'
#define SUBTRACT '-'
#define MULTIPLY '*'
#define DIVIDE '/'
#define MODULO '%'
#define EXPONENTIATION "**"
// ops: variables
#define ASSIGNMENT '='
// ops: boolean
// prefixed with BOOL to avoid any possible conflicts in the future
#define BOOL_TRUE "true"
#define BOOL_FALSE "false"

// expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

#define COMMENT '#'

// currently unsupported
// #define BANG '!'

enum : uint8_t {
    None = 0,
    IntLit,
    FloatLit,
    StrLit,
    GlobStar,
    GlobQuestion,

};

typedef struct {
    char* line;
    size_t len;

    size_t pos;

    char* tok;
    // size_t tok_len;
    uint8_t tok_type;
} Lexer;

void lexer_init(Lexer* lex, char* r line, size_t len, Arena* r temp);

int lexer_lex(Lexer* lex);
