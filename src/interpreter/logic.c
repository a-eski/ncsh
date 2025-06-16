/* Copyright ncsh (C) by Alex Eski 2025 */
/* logic.c: Preprocessing logic/control flow structures specifically to ensure ready for VM to process. */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "logic.h"

#define DEFAULT_N 10
#define MAX_N 40

void logic_tokens_conditions_init(Token_Data* rst data, Arena* rst scratch)
{
    assert(data);
    data->conditions = arena_malloc(scratch, 1, Statements);
    data->conditions->cap = DEFAULT_N;
    data->conditions->count = 0;
    data->conditions->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    data->conditions->commands->cap = DEFAULT_N;
    data->conditions->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        data->conditions->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        data->conditions->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        data->conditions->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        data->conditions->commands[i].cap = DEFAULT_N;
        data->conditions->commands[i].count = 0;
    }
}

void logic_tokens_if_init(Token_Data* rst data, Arena* rst scratch)
{
    assert(data);
    data->if_statements = arena_malloc(scratch, 1, Statements);
    data->if_statements->cap = DEFAULT_N;
    data->if_statements->count = 0;
    data->if_statements->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    data->if_statements->commands->cap = DEFAULT_N;
    data->if_statements->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        data->if_statements->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        data->if_statements->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        data->if_statements->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        data->if_statements->commands[i].cap = DEFAULT_N;
        data->if_statements->commands[i].count = 0;
    }
}

void logic_tokens_else_init(Token_Data* rst data, Arena* rst scratch)
{
    data->else_statements = arena_malloc(scratch, 1, Statements);
    data->else_statements->cap = DEFAULT_N;
    data->else_statements->count = 0;
    data->else_statements->commands = arena_malloc(scratch, DEFAULT_N, Commands);
    data->else_statements->commands->cap = DEFAULT_N;
    data->else_statements->commands->count = 0;

    for (size_t i = 0; i < DEFAULT_N; ++i) {
        data->else_statements->commands[i].vals = arena_malloc(scratch, DEFAULT_N, char*);
        data->else_statements->commands[i].lens = arena_malloc(scratch, DEFAULT_N, size_t);
        data->else_statements->commands[i].ops = arena_malloc(scratch, DEFAULT_N, enum Ops);
        data->else_statements->commands[i].cap = DEFAULT_N;
        data->else_statements->commands[i].count = 0;
    }
}

void logic_commands_add(Token* rst tok, Commands* rst cmds, Arena* rst scratch)
{
    assert(tok);
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

    cmds->vals[cmds->count] = arena_malloc(scratch, tok->len, char);
    memcpy(cmds->vals[cmds->count], tok->val, tok->len);
    cmds->lens[cmds->count] = tok->len;
    cmds->ops[cmds->count] = tok->op;
    ++cmds->count;
}

void logic_statements_add(Token* rst tok, Statements* statements, Arena* rst scratch)
{
    assert(tok);
    assert(statements);
    assert(statements->cap > 0);

    if (statements->count == statements->cap) {
        size_t c = statements->cap;
        statements->cap *= 2;
        if (statements->cap > MAX_N)
            return;
        statements->commands = arena_realloc(scratch, statements->cap, Commands, statements->commands, c);
    }

    logic_commands_add(tok, &statements->commands[statements->count], scratch);
}

[[nodiscard]]
bool logic_is_tok_valid_statement(Token* tok)
{
    if (!tok)
        return false;

    switch (tok->op) {
    case OP_CONSTANT:
    case OP_TRUE:
    case OP_FALSE:
    case OP_EQUALS:
    case OP_AND:
    case OP_OR:
    case OP_LESS_THAN:
    case OP_GREATER_THAN: {
        return true;
    }
    default: {
        return false;
    }
    }
}

[[nodiscard]]
Token* logic_statements_process(Token* rst tok, Statements* rst statements, Arena* rst scratch)
{
    do {
        bool end_of_statement = false;
        if (tok->len > 2 && tok->val[tok->len - 2] == ';') {
            tok->val[tok->len - 2] = '\0';
            --tok->len;
            end_of_statement = true;
        }
        debugf("adding token to statements %s\n", tok->val);
        logic_statements_add(tok, statements, scratch);
        if (end_of_statement)
            ++statements->count;
        tok = tok->next;
    } while (logic_is_tok_valid_statement(tok));

    if (statements->count == 0)
        statements->count = 1;
    return tok;
}

[[nodiscard]]
Logic_Result logic_if_preprocess(Token* tok, Token_Data* data, Arena* rst scratch)
{
    tok = tok->next;
    if (tok->op != OP_CONDITION_START)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};
    tok = tok->next;

    if (!logic_is_tok_valid_statement(tok))
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    debug("processing conditions");
    logic_tokens_conditions_init(data, scratch);
    tok = logic_statements_process(tok, data->conditions, scratch);

    if (tok->op != OP_CONDITION_END)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    tok = tok->next;
    if (tok->op != OP_THEN)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    tok = tok->next;
    if (!logic_is_tok_valid_statement(tok))
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    debug("processing if statements");
    logic_tokens_if_init(data, scratch);
    tok = logic_statements_process(tok, data->if_statements, scratch);

    if (tok->op == OP_FI) {
        return (Logic_Result){.type = LT_IF, .val.tok = tok};
    }

    /*if (tok->op != OP_ELSE || tok->op != OP_ELIF)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    while (tok->op == OP_ELIF) {
        if (tok->op == OP_ELIF) {
            tok = tok->next;
            if (!logic_is_tok_valid_statement(tok))
                return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

            debug("processing else statements");
            logic_tokens_else_init(tokens, scratch);
            tok = logic_statements_process(tok, tokens->else_statements, scratch);
        }
    }*/

    if (tok->op != OP_ELSE)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    tok = tok->next;
    if (!logic_is_tok_valid_statement(tok))
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    debug("processing else statements");
    logic_tokens_else_init(data, scratch);
    tok = logic_statements_process(tok, data->else_statements, scratch);

    if (tok->op != OP_FI)
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};

    if (tok->next)
        return (Logic_Result){.type = LT_IF_ELSE, .val.tok = tok->next};
    else
        return (Logic_Result){.type = LT_IF_ELSE, .val.tok = tok};
}

[[nodiscard]]
Logic_Result logic_preprocess(Token* rst tok, Token_Data* rst data, Arena* rst scratch)
{
    assert(tok);
    assert(scratch);

    if (!tok) {
        puts("ncsh: logic processing failed, NULL token passed in.");
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};
    }

    switch (tok->op) {
    case OP_IF: {
        return logic_if_preprocess(tok, data, scratch);
    }
    default: {
        puts("ncsh: an unsupported operation was found while trying to process control flow logic (i.e. 'if', 'while', "
             "'for', etc.).");
        return (Logic_Result){.type = LT_CODE, .val.code = EXIT_FAILURE_CONTINUE};
    }
    }
}
