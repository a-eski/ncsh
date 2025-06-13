#pragma once

#define STRCMP(line, expected_lit) line[0] == expected_lit[0] && !memcmp(line, expected_lit, sizeof(expected_lit) - 1)

// supported quotes
#define SINGLE_QUOTE_KEY '\''
#define DOUBLE_QUOTE_KEY '\"'
#define SINGLE_QUOTE_KEY '\''
#define BACKTICK_QUOTE_KEY '`'

// symbols (unused currently)
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

// ops: expansions
#define GLOB_STAR '*'
#define GLOB_QUESTION '?'
#define TILDE '~'

// ops: control flow structures
#define IF "if"
#define FI "fi"
#define DO "do"
#define THEN "then"
#define ELSE "else"
#define ELIF "elif"
// #define SEMICOLON ';'
// #define OPENING_BRACKET '['
// #define CLOSING_BRACKET ']'
#define CONDITION_START '['
#define CONDITION_END "];"

// ops: equality
#define EQUALS "-eq"
#define LESS_THAN "-lt"
#define GREATER_THAN "-gt"

// ops: misc
#define COMMENT '#'

// currently unsupported
#define BANG '!'
