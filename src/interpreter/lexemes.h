/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <unistd.h>
#include <stdint.h>

#include "../arena.h"

#define LEXER_TOKENS_LIMIT 128

typedef struct {
    size_t count;
    uint8_t* ops;
    size_t* lens;
    char** vals;
} Lexemes;

void lexemes_init(Lexemes* rst lexemes, Arena* rst scratch);
