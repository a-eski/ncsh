/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "lex.h"
#include "stmts.h"

enum {
    PE_NOTHING = 1,
    PE_INVALID_STMT,
    PE_MISSING_TOK
};

typedef struct {
    int parser_errno;
    union {
        char* msg;
        Statements* stmts;
    } output;
} Parser_Output;

Parser_Output parse(Lexemes* lexemes, Arena* restrict scratch);
