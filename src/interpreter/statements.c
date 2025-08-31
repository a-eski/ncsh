#include <assert.h>
#include <stdint.h>

#include "../arena.h"
#include "statements.h"

#define STMT_DEFAULT_N 10
#define STMT_MAX_N 40

Commands* commands_alloc(Arena* restrict scratch)
{
    Commands* c = arena_malloc(scratch, 1, Commands);
    c->count = 0;
    c->cap = STMT_DEFAULT_N;
    c->strs = arena_malloc(scratch, STMT_DEFAULT_N, Str);
    c->ops = arena_malloc(scratch, STMT_DEFAULT_N, enum Ops);
    c->next = NULL;
    c->prev_op = OP_NONE;
    return c;
}

void command_realloc(Commands* restrict cmds, Arena* restrict scratch)
{
    size_t c = cmds->cap;
    size_t new_cap = c *= 2;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

void commands_realloc(Statements* restrict stmts, Arena* restrict scratch)
{
    size_t c = stmts->statements[stmts->count].commands->cap;
    size_t new_cap = c *= 2;
    stmts->statements[stmts->count].commands->cap = new_cap;
    stmts->statements[stmts->count].commands->strs =
        arena_realloc(scratch, new_cap, Str, stmts->statements[stmts->count].commands->strs, c);
    stmts->statements[stmts->count].commands->ops =
        arena_realloc(scratch, new_cap, enum Ops, stmts->statements[stmts->count].commands->ops, c);
}

Commands* command_next(Commands* restrict cmds, Arena* restrict scratch)
{
    if (!cmds->pos) {
        cmds->count = 1;
        cmds->strs[1].value = NULL;
    }
    else {
        cmds->count = cmds->pos;
    }

    cmds->next = commands_alloc(scratch);
    cmds->pos = 0;

    cmds = cmds->next;
    cmds->pos = 0;
    return cmds;
}

Commands* command_statement_next(Statements* restrict stmts, Commands* cmds, enum Logic_Type type, Arena* restrict scratch)
{
    cmds->count = cmds->pos == 0 ? 1 : cmds->pos; // update last commands count

    statement_next(stmts, type, scratch);

    return stmts->statements[stmts->pos].commands;
}

void statements_init(Statements* restrict stmts, Arena* restrict scratch)
{
    assert(stmts);
    stmts->statements = arena_malloc(scratch, STMT_DEFAULT_N, Statement);
    stmts->type = ST_NORMAL;
    stmts->statements->count = 0;
    stmts->cap = STMT_DEFAULT_N;
    stmts->statements->type = LT_NORMAL;
    stmts->statements->commands = commands_alloc(scratch);
}

void statement_next(Statements* restrict stmts, enum Logic_Type type, Arena* restrict scratch)
{
    stmts->statements[stmts->pos].type = type;
    ++stmts->statements[stmts->pos].count;
    ++stmts->pos;
    ++stmts->count;
    stmts->cap = STMT_DEFAULT_N;
    stmts->statements[stmts->pos].commands = commands_alloc(scratch);
}
