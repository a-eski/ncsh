/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>

#include "lexemes.h"

void lexemes_init(Lexemes* rst lexemes, Arena* rst scratch)
{
    assert(lexemes);
    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->lens = arena_malloc(scratch, LEXER_TOKENS_LIMIT, size_t);
    lexemes->vals = arena_malloc(scratch, LEXER_TOKENS_LIMIT, char*);
}

Lexemes* lexemes_alloc(Arena* rst scratch)
{
    Lexemes* lexemes = arena_malloc(scratch, 1, Lexemes);
    lexemes->count = 0;
    lexemes->ops = arena_malloc(scratch, LEXER_TOKENS_LIMIT, uint8_t);
    lexemes->lens = arena_malloc(scratch, LEXER_TOKENS_LIMIT, size_t);
    lexemes->vals = arena_malloc(scratch, LEXER_TOKENS_LIMIT, char*);
    return lexemes;
}
