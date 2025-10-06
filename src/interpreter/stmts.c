/* Copyright ncsh (C) by Alex Eski 2025 */
/* stmts.h: statements and commands for ncsh */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "../arena.h"
#include "../defines.h"
#include "stmts.h"

#define STMT_DEFAULT_N 25
#define STMT_MAX_N 100

Commands* cmds_alloc(Arena* restrict scratch)
{
    Commands* c = arena_malloc(scratch, 1, Commands);
    c->count = 0;
    c->cap = STMT_DEFAULT_N;
    c->strs = arena_malloc(scratch, STMT_DEFAULT_N, Str);
    c->ops = arena_malloc(scratch, STMT_DEFAULT_N, enum Ops);
    c->next = NULL;
    c->op = OP_NONE;
    c->prev_op = OP_NONE;
    return c;
}

void cmd_realloc(Commands* restrict cmds, Arena* restrict scratch)
{
    size_t c = cmds->cap;
    size_t new_cap = c *= 2;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

void cmd_realloc_exact(Commands* restrict cmds, Arena* restrict scratch, size_t new_cap)
{
    size_t c = cmds->cap;
    cmds->cap = new_cap;
    cmds->strs =
        arena_realloc(scratch, new_cap, Str, cmds->strs, c);
    cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, cmds->ops, c);
}

void cmds_realloc(Parser_Data* restrict data, Arena* restrict scratch)
{
    size_t c = data->cur_cmds->cap;
    size_t new_cap = c *= 2;
    data->cur_cmds->cap = new_cap;
    data->cur_cmds->strs =
        arena_realloc(scratch, new_cap, Str, data->cur_cmds->strs, c);
    data->cur_cmds->ops =
        arena_realloc(scratch, new_cap, enum Ops, data->cur_cmds->ops, c);
}

Commands* cmd_next(Commands* restrict cmds, Arena* restrict scratch)
{
    if (!cmds->pos) {
        cmds->count = 1;
        cmds->strs[1].value = NULL;
    }
    else {
        cmds->count = cmds->pos;
    }

    cmds->next = cmds_alloc(scratch);
    cmds->pos = 0;

    cmds = cmds->next;
    cmds->pos = 0;
    return cmds;
}

Statement* stmt_alloc(Arena* restrict scratch)
{
    Statement* stmt = arena_malloc(scratch, 1, Statement);
    stmt->type = LT_NORMAL;
    stmt->commands = cmds_alloc(scratch);
    return stmt;
}

int stmt_next(Parser_Data* restrict data, enum Logic_Type type)
{
    if (!data->stmts->head) {
        assert(data->cur_stmt);
        data->cur_stmt->type = type;
        data->stmts->head = data->cur_stmt;
        data->cur_stmt = stmt_alloc(data->s);
        data->prev_stmt = data->stmts->head;
        return EXIT_SUCCESS;
    }

    switch (type) {
    case LT_NORMAL:
    case LT_IF_CONDITIONS:
    case LT_IF:
    case LT_ELIF:
    case LT_WHILE_CONDITIONS:
    case LT_WHILE:
        goto right;
    case LT_ELIF_CONDITIONS:
    case LT_ELSE:
        goto left;
    }

    unreachable();
    return EXIT_FAILURE_CONTINUE;

right:
    data->cur_stmt->type = type;
    data->cur_stmt->prev = data->prev_stmt;
    data->cur_stmt->prev->right = data->cur_stmt;
    data->prev_stmt = data->cur_stmt;
    data->cur_stmt = stmt_alloc(data->s);
    return EXIT_SUCCESS;

left:
    data->cur_stmt->type = type;
    data->cur_stmt->prev = data->prev_stmt->prev;
    data->cur_stmt->prev->left = data->cur_stmt;
    data->prev_stmt = data->cur_stmt;
    data->cur_stmt = stmt_alloc(data->s);
    return EXIT_SUCCESS;
}

void cmd_stmt_next(Parser_Data* data, enum Logic_Type type)
{
    data->cur_cmds->count = data->cur_cmds->pos == 0 ? 1 : data->cur_cmds->pos; // update last commands count

    stmt_next(data, type);

    data->cur_cmds = data->cur_stmt->commands;
}

Statements* stmts_alloc(Arena* restrict scratch)
{
    Statements* stmts = arena_malloc(scratch, 1, Statements);
    assert(stmts);
    stmts->head = arena_malloc(scratch, STMT_DEFAULT_N, Statement);
    stmts->type = ST_NORMAL;
    stmts->head->type = LT_NORMAL;
    stmts->head->commands = cmds_alloc(scratch);
    return stmts;
}
