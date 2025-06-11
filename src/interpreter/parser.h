/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "../types.h"
#include "interpreter_types.h"

int parser_parse(Tokens* rst toks, Shell* rst shell, Arena* rst scratch);
