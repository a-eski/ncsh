/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "tokens.h"
#include "../types.h"
#include "vm/vm_types.h"

int parser_parse(Tokens* rst toks, Token_Data* rst data, Shell* rst shell, Arena* rst scratch);
