/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "../types.h"
#include "lexemes.h"
#include "stmts.h"

enum {
    PE_NOTHING,
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

Parser_Output parser_parse(Lexemes* lexemes, Shell* restrict shell, Arena* restrict scratch);
