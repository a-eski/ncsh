/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "../types.h"
#include "lexemes.h"
#include "stmts.h"

void expansion_home(Parser_Data* restrict data, size_t pos);

void expansion_glob(char* restrict in, Commands* restrict cmds, Arena* restrict scratch);

void expansion_assignment(Lexemes* lexeme, size_t pos, Shell* restrict shell);

void expansion_variable(Parser_Data* restrict data, Str* restrict in);

void expansion_alias(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch);
