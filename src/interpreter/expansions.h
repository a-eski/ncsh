/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../types.h"
#include "lexemes.h"
#include "statements.h"

void expansion_home(Shell* shell, Lexemes* restrict lexemes, size_t pos, Arena* scratch);

void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch);

void expansion_assignment(Lexemes* lexeme, size_t pos, Vars* restrict vars, Arena* restrict arena);

void expansion_variable(char* restrict in, size_t len, Commands* restrict cmds, /*Statements* stmts,*/ Shell* shell, Arena* scratch);

void expansion_alias(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch);
