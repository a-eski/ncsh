/* Copyright ncsh (C) by Alex Eski 2025 */
/* args.c: a linked list for storing tokens outputted by the lexer */

#include <assert.h>
#include <string.h>

#include "../arena.h"
#include "tokens.h"
#include "ops.h"

[[nodiscard]]
Tokens* tokens_alloc(Arena* rst arena)
{
    Tokens* toks = arena_malloc(arena, 1, Tokens);
    toks->count = 0;
    // allocate the first head as empty with no op (0 is OP_NONE)
    toks->head = token_alloc(OP_NONE, 1, "\0", arena);
    return toks;
}

[[nodiscard]]
Token* token_alloc(uint8_t op, size_t len, char* rst val, Arena* rst arena)
{
    Token* tok = arena_malloc(arena, 1, Token);
    tok->val = arena_malloc(arena, len, char);
    memcpy(tok->val, val, len);
    tok->op = op;
    tok->len = len;
    tok->next = NULL;
    return tok;
}

[[nodiscard]]
bool token_set_after(Token* rst current, Token* rst after)
{
    assert(current);
    assert(after);
    if (!current || !after)
        return false;

    Token* temp = current->next;
    current->next = after;
    after->next = temp;

    return true;
}

[[nodiscard]]
bool token_set_last(Tokens* rst toks, Token* rst last)
{
    assert(toks && toks->head);
    assert(last);
    if (!toks || !last)
        return false;

    Token* tok = toks->head;

    while (tok->next)
        tok = tok->next;

    tok->next = last;
    last->next = NULL;
    ++toks->count;

    return true;
}
