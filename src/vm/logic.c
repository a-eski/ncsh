/* Copyright ncsh (C) by Alex Eski 2025 */
/* logic.c: Preprocessing logic/control flow structures specifically to ensure ready for VM to process. */

#include "logic.h"
#include "vm_types.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_N 10
#define MAX_N 40

void logic_commands_init(Commands* rst cmds, Arena* rst scratch)
{
    cmds = arena_malloc(scratch, 1, Commands);
    cmds->vals = arena_malloc(scratch, DEFAULT_N, char*);
    cmds->lens = arena_malloc(scratch, DEFAULT_N, size_t);
    cmds->ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
    cmds->cap = DEFAULT_N;
    cmds->count = 0;
}

void logic_statements_init(Statements* rst statements, Arena* rst scratch)
{
    statements = arena_malloc(scratch, 1, Statements);
    statements->cap = DEFAULT_N;
    statements->count = 0;
    statements->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    statements->commands->cap = DEFAULT_N;
    statements->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        statements->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        statements->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        statements->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        statements->commands[i].cap = DEFAULT_N;
        statements->commands[i].count = 0;
    }
}

void logic_tokens_if_init(Token_Data* rst tokens, Arena* rst scratch)
{
    assert(tokens);
    tokens->conditions = arena_malloc(scratch, 1, Commands);
    tokens->conditions->vals = arena_malloc(scratch, DEFAULT_N, char*);
    tokens->conditions->lens = arena_malloc(scratch, DEFAULT_N, size_t);
    tokens->conditions->ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
    tokens->conditions->cap = DEFAULT_N;
    tokens->conditions->count = 0;

    tokens->if_statements = arena_malloc(scratch, 1, Statements);
    tokens->if_statements->cap = DEFAULT_N;
    tokens->if_statements->count = 0;
    tokens->if_statements->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    tokens->if_statements->commands->cap = DEFAULT_N;
    tokens->if_statements->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        tokens->if_statements->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        tokens->if_statements->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        tokens->if_statements->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        tokens->if_statements->commands[i].cap = DEFAULT_N;
        tokens->if_statements->commands[i].count = 0;
    }
}

void logic_tokens_else_init(Token_Data* rst tokens, Arena* rst scratch)
{
    tokens->else_statements = arena_malloc(scratch, 1, Statements);
    tokens->else_statements->cap = DEFAULT_N;
    tokens->else_statements->count = 0;
    tokens->else_statements->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    tokens->else_statements->commands->cap = DEFAULT_N;
    tokens->else_statements->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        tokens->else_statements->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        tokens->else_statements->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        tokens->else_statements->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        tokens->else_statements->commands[i].cap = DEFAULT_N;
        tokens->else_statements->commands[i].count = 0;
    }
}

void logic_commands_add(Arg* rst arg, Commands* rst cmds, Arena* rst scratch)
{
    assert(arg);
    assert(cmds);
    assert(cmds->cap);

    if (cmds->count == cmds->cap) {
        size_t c = cmds->cap;
        cmds->cap *= 2;
        if (cmds->cap > MAX_N)
            return;
        cmds->vals = arena_realloc(scratch, cmds->cap, char*, cmds->vals, c);
        cmds->lens = arena_realloc(scratch, cmds->cap, size_t, cmds->lens, c);
        cmds->ops = arena_realloc(scratch, cmds->cap, enum Ops, cmds->ops, c);
    }

    cmds->vals[cmds->count] = arena_malloc(scratch, arg->len, char);
    memcpy(cmds->vals[cmds->count], arg->val, arg->len);
    cmds->lens[cmds->count] = arg->len;
    cmds->ops[cmds->count] = arg->op;
    ++cmds->count;
}

void logic_statements_add(Arg* rst arg, Statements* statements, Arena* rst scratch)
{
    assert(arg);
    assert(statements);
    assert(statements->cap > 0);

    if (statements->count == statements->cap) {
        size_t c = statements->cap;
        statements->cap *= 2;
        if (statements->cap > MAX_N)
            return;
        statements->commands = arena_realloc(scratch, statements->cap, Commands, statements->commands, c);
    }

    logic_commands_add(arg, &statements->commands[statements->count], scratch);
}

bool logic_is_arg_valid_statement(Arg* arg)
{
    if (!arg)
        return false;
    switch (arg->op) {
    case OP_CONSTANT:
    case OP_TRUE:
    case OP_FALSE:
    case OP_EQUALS:
    case OP_LESS_THAN:
    case OP_GREATER_THAN: {
        return true;
    }
    default: {
        return false;
    }
    }
}

Arg* logic_statements_process(Arg* rst arg, Statements* rst statements, Arena* rst scratch)
{
    do {
        bool end_of_statement = false;
        if (arg->len > 2 && arg->val[arg->len - 2] == ';') {
            arg->val[arg->len - 2] = '\0';
            --arg->len;
            end_of_statement = true;
        }
        debugf("adding arg to statements %s\n", arg->val);
        logic_statements_add(arg, statements, scratch);
        if (end_of_statement)
            ++statements->count;
        arg = arg->next;
    } while (logic_is_arg_valid_statement(arg));

    return arg;
}

Logic_Result logic_if_preprocess(Arg* arg, Token_Data* rst tokens, Arena* rst scratch)
{
    logic_tokens_if_init(tokens, scratch);

    arg = arg->next;
    if (arg->op != OP_CONDITION_START)
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};
    arg = arg->next;

    if (!logic_is_arg_valid_statement(arg))
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    do {
        debugf("adding arg to conditions %s\n", arg->val);
        logic_commands_add(arg, tokens->conditions, scratch);
        arg = arg->next;
    } while (logic_is_arg_valid_statement(arg));

    if (arg->op != OP_CONDITION_END)
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    arg = arg->next;
    if (arg->op != OP_THEN)
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    arg = arg->next;
    if (!logic_is_arg_valid_statement(arg))
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    debug("processing if statements");
    arg = logic_statements_process(arg, tokens->if_statements, scratch);

    if (arg->op == OP_FI) {
        return (Logic_Result){.type = LT_IF, .val.arg = arg};
    }

    if (arg->op != OP_ELSE)
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    arg = arg->next;
    if (!logic_is_arg_valid_statement(arg))
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    debug("processing else statements");
    logic_tokens_else_init(tokens, scratch);
    arg = logic_statements_process(arg, tokens->else_statements, scratch);

    if (arg->op != OP_FI)
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};

    if (arg->next)
        return (Logic_Result){.type = LT_IF_ELSE, .val.arg = arg->next};
    else
        return (Logic_Result){.type = LT_IF_ELSE, .val.arg = arg};
}

Logic_Result logic_preprocess(Arg* rst arg, Token_Data* rst tokens, Arena* rst scratch)
{
    assert(arg);
    assert(tokens);
    assert(scratch);

    if (!arg) {
        puts("ncsh: logic processing failed, NULL arg passed in.");
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};
    }

    switch (arg->op) {
    case OP_IF: {
        return logic_if_preprocess(arg, tokens, scratch);
    }
    default: {
        puts("ncsh: an unsupported operation was found while trying to process control flow logic (i.e. 'if', 'while', "
             "'for', etc.).");
        return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};
    }
    }
}

/*
 if (arg->op != OP_CONSTANT) {
        enum Ops op = arg->op;
        arg = arg->next;
        assert(arg);

        char* c2 = arg->val;
        arg = arg->next;
        assert(arg);

        bool result;
        switch (op) {
        case OP_EQUALS: {
            result = atoi(c1->val) == atoi(c2);
            break;
        }
        case OP_LESS_THAN: {
            result = atoi(c1->val) < atoi(c2);
            break;
        }
        case OP_GREATER_THAN: {
            result = atoi(c1->val) > atoi(c2);
            break;
        }
        default: {
            puts("ncsh: while trying to process 'if' logic, found unsupported operation.");
            result = false;
            break;
        }
        }

        if (!result)
            return (Logic_Result){.type = LT_CODE, .val.code = NCSH_COMMAND_FAILED_CONTINUE};
    }
 */
