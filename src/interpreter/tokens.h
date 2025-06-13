/* Copyright ncsh (C) by Alex Eski 2025 */
/* args.h: a linked list for storing tokens outputted by the lexer */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../arena.h"

typedef struct Token_ {
    uint8_t op;
    size_t pos;
    size_t count;
    size_t* lens;
    char** vals;
    struct Token_* next;
} Token;

typedef struct {
    size_t count;
    Token* head;
} Tokens;

Tokens* tokens_alloc(Arena* rst arena);

Token* token_alloc(uint8_t op, size_t len, char* rst val, Arena* rst arena);

bool token_set_after(Token* rst current, Token* rst after);

bool token_set_last(Tokens* rst toks, Token* rst last);
