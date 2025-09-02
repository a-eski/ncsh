/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "../types.h"
#include "lexemes.h"
#include "stmts.h"

/* Parser errnos */
#define PE_NOTHING -1 // nothing to process

typedef struct {
    int parser_errno;
    char* msg;
    Statements* stmts;
} Parser_Output;

// int parser_parse(Lexemes* lexemes, Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch);
Parser_Output parser_parse(Lexemes* lexemes, Shell* restrict shell, Arena* restrict scratch);
