/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <stdint.h>

#include "lexemes.h"

void lexemes_init(Lexemes* restrict lexemes, Arena* restrict scratch)
{
    assert(lexemes);

    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->lens = arena_malloc(scratch, LEXER_TOKENS_LIMIT, size_t);
    lexemes->vals = arena_malloc(scratch, LEXER_TOKENS_LIMIT, char*);
}

void lexemes_init_n(Lexemes* restrict lexemes, size_t n, Arena* restrict scratch)
{
    assert(lexemes);
    assert(n);

    [[maybe_unused]] uint8_t* ops = lexemes->ops + n;
    ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);

    [[maybe_unused]] size_t* lens = lexemes->lens + n;
    lens = arena_malloc(scratch, LEXER_TOKENS_LIMIT, size_t);

    [[maybe_unused]] char** vals = lexemes->vals + n;
    vals = arena_malloc(scratch, LEXER_TOKENS_LIMIT, char*);
}

/*Lexemes* lexemes_alloc(Arena* restrict scratch)
{
    Lexemes* lexemes = arena_malloc(scratch, 1, Lexemes);
    lexemes->count = 0;
    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->lens = arena_malloc(scratch, LEXER_TOKENS_LIMIT, size_t);
    lexemes->vals = arena_malloc(scratch, LEXER_TOKENS_LIMIT, char*);
    return lexemes;
}*/
