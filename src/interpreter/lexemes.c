/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <stdint.h>

#include "lexemes.h"

void lexemes_init(Lexemes* restrict lexemes, Arena* restrict scratch)
{
    assert(lexemes);

    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->strs = arena_malloc(scratch, LEXER_TOKENS_LIMIT, Str);
}

void lexemes_init_n(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch)
{
    assert(lexemes);
    assert(n);

    [[maybe_unused]] uint8_t* ops = lexemes->ops + n;
    ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);

    [[maybe_unused]] Str* strs = lexemes->strs + n;
    strs = arena_malloc(scratch, LEXER_TOKENS_LIMIT, Str);
}
