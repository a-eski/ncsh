/* Copyright ncsh (C) by Alex Eski 2025 */
/* args.h: a linked list for storing tokens outputted by the lexer */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../arena.h"
#include "interpreter_types.h"

Tokens* tokens_alloc(Arena* rst arena);

Token* token_alloc(uint8_t op, size_t len, char* rst val, Arena* rst arena);

bool token_set_after(Token* rst current, Token* rst after);

bool token_set_last(Tokens* rst toks, Token* rst last);
