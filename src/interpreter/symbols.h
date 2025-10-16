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
#define TILDE '~'
#define EQ '='
#define AMP '&'
#define PLUS '+'
#define MINUS '-'
#define MOD '%'
#define FSLASH '/'

#define EXP "**"

// ops: boolean
// prefixed with BOOL to avoid any possible conflicts in the future
#define BOOL_TRUE "true"
#define BOOL_FALSE "false"

// ops: control flow structures
#define IF "if"
#define FI "fi"
#define DO "do"
#define THEN "then"
#define ELSE "else"
#define ELIF "elif"

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
