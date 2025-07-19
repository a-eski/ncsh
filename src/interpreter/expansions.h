/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../shell.h"
#include "lexemes.h"
#include "statements.h"

void expansion_home(Lexemes* rst lexemes, size_t pos, Arena* rst scratch);

void expansion_glob(char* rst in, Commands* rst cmds, Arena* rst scratch);

void expansion_assignment(Lexemes* lexeme, size_t pos, Vars* rst vars, Arena* rst arena);

void expansion_variable(char* rst in, size_t len, Commands* rst cmds, /*Statements* stmts,*/ Shell* rst shell, Arena* rst scratch);

void expansion_alias(Lexemes* rst lexemes, size_t n, Arena* rst scratch);
