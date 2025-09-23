#pragma once

// supported quotes
#define SINGLE_QUOTE '\''
#define DOUBLE_QUOTE '\"'
#define BACKTICK_QUOTE '`'

// symbols
#define O_PARAN '('
#define C_PARAN ')'
#define PIPE '|'
#define GT '>'
#define LT '<'
#define O_BRACKET '['
#define C_BRACKET ']'
#define SEMICOLON ';'
#define COMMENT '#'
#define BANG '!'
#define DOLLAR '$'
#define STAR '*'
#define QUESTION '?'
#define EQ '='
#define AMP '&'

// TODO: remove unused
#define STDIN_REDIRECTION '<'
#define STDOUT_REDIRECTION '>'
#define BACKGROUND_JOB '&'

#define STDIN_REDIRECTION_APPEND "<<"
#define STDOUT_REDIRECTION_APPEND ">>"
#define STDERR_REDIRECTION "2>"
#define STDERR_REDIRECTION_APPEND "2>>"
#define STDOUT_AND_STDERR_REDIRECTION "&>"
#define STDOUT_AND_STDERR_REDIRECTION_APPEND "&>>"
#define AND "&&"
#define OR "||"

// ops: multiple
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
#define CONDITION_START '['
#define CONDITION_END "];"
#define CONDITION_START_ALT "[["
#define CONDITION_END_ALT "]];"

// ops: equality
#define EQUALS "-eq"
#define LESS_THAN "-lt"
#define LESS_THAN_OR_EQUALS "-le"
#define GREATER_THAN "-gt"
#define GREATER_THAN_OR_EQUALS "-ge"

// ops: loops
#define WHILE "while"
#define FOR "for"
#define DO "do"
#define DONE "done"
