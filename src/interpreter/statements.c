#include <assert.h>
#include <stdint.h>

#include "statements.h"
#include "../defines.h"
#include "../arena.h"

#define DEFAULT_N 10
#define MAX_N 40

Commands* commands_alloc(Arena* rst scratch)
{
    Commands* commands = arena_malloc(scratch, 1, Commands);
    commands->count = 0;
    commands->cap = DEFAULT_N;
    commands->vals = arena_malloc(scratch, DEFAULT_N, char*);
    commands->lens = arena_malloc(scratch, DEFAULT_N, size_t);
    commands->ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
    commands->next = NULL;
    return commands;
}

void commands_realloc(Statements* rst statements, Arena* rst scratch)
{
    size_t cap = statements->statements[statements->count].commands->cap;
    size_t new_cap = cap *= 2;
    statements->statements[statements->count].commands->cap = new_cap;
    statements->statements[statements->count].commands->vals =
        arena_realloc(scratch, new_cap, char*, statements->statements[statements->count].commands->vals, cap);
    statements->statements[statements->count].commands->lens =
        arena_realloc(scratch, new_cap, size_t, statements->statements[statements->count].commands->lens, cap);
    statements->statements[statements->count].commands->ops =
        arena_realloc(scratch, new_cap, enum Ops, statements->statements[statements->count].commands->ops, cap);
}

void statements_init(Statements* rst statements, Arena* rst scratch)
{
    assert(statements);
    statements->statements = arena_malloc(scratch, DEFAULT_N, Statement);
    statements->statements->count = 0;
    statements->statements->type = ST_NORMAL;
    statements->statements->commands = commands_alloc(scratch);
}
