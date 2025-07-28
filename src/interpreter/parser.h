/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "../shell.h"
#include "lexemes.h"
#include "statements.h"

int parser_parse(Lexemes* lexemes, Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch);
