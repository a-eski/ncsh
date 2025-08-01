/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "../arena.h"

#define LEXER_TOKENS_LIMIT 128

typedef struct {
    uint8_t op;
    size_t len;
    char* val;
} Lexeme;

typedef struct {
    size_t count;
    uint8_t* ops;
    size_t* lens;
    char** vals;
} Lexemes;

void lexemes_init(Lexemes* restrict lexemes, Arena* restrict scratch);

void lexemes_init_n(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch);
