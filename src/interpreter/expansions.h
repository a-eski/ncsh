/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "lexemes.h"
#include "../types.h"
#include "statements.h"

void expansion_home(Lexemes* rst lexemes, size_t pos, Arena* rst scratch);

bool expansion_glob(Lexemes* rst lexeme, size_t pos, Arena* rst scratch);

void expansion_assignment(Lexemes* lexeme, size_t pos, Vars* rst vars, Arena* rst arena);
