/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_buffer.c: setup commands in a form that can be sent to execvp/execve/etc. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../tokens.h"
#include "vm_types.h"

Token* vm_buffer_set_command_next(Token* rst tok, Vm_Data* rst vm)
{
    if (tok && tok->op == OP_ASSIGNMENT) {
        tok = tok->next;
        if (tok && (tok->op == OP_AND || tok->op == OP_OR))
            tok = tok->next;
    }

    if (!tok) {
        vm->buffer[0] = NULL;
        vm->tokens_end = true;
        return NULL;
    }

    size_t vm_buf_pos = 0;
    while (tok && tok->val) {
        if (tok->op == OP_PIPE || tok->op == OP_AND || tok->op == OP_OR)
            break;
        assert(tok->val[tok->len - 1] == '\0');
        vm->buffer[vm_buf_pos] = tok->val;
        vm->buffer_lens[vm_buf_pos] = tok->len;
        debugf("set vm->buffer[%zu] to %s, %zu\n", vm_buf_pos, arg->val, arg->len);

        if (!tok->next || !tok->next->val) {
            vm->tokens_end = true;
            ++vm_buf_pos;
            break;
        }

        ++vm_buf_pos;
        tok = tok->next;
    }

    if (!vm->tokens_end) {
        vm->op_current = tok->op;
        debugf("set op current to %d\n", vm->op_current);
        tok = tok->next;
    }

    vm->buffer[vm_buf_pos] = NULL;

    return tok;
}

void vm_buffer_set_if(Token_Data* rst data, Vm_Data* rst vm)
{
    switch (vm->state) {
    case VS_NORMAL: {
        goto conditions;
    }
    // conditions just processed, decide what to do next
    case VS_IN_CONDITIONS: {
        if (vm->conditions_pos < data->conditions->count - 1)
            goto conditions;
        else if (data->logic_type == LT_IF || vm->status == EXIT_SUCCESS)
            goto if_statements;
        else
            goto else_statements;
    }
    case VS_IN_IF_STATEMENTS: {
        if (vm->if_statment_pos < data->if_statements->count - 1)
            goto if_statements;
        else
            goto end;
    }
    case VS_IN_ELSE_STATEMENTS: {
        if (vm->else_statment_pos < data->else_statements->count - 1)
            goto else_statements;
        else
            goto end;
    }
    default: {
        goto end;
    }
    }

conditions:
    debug("setting conditions");
    vm->buffer = data->conditions->commands[vm->conditions_pos].vals;
    vm->buffer_lens = data->conditions->commands[vm->conditions_pos].lens;
    vm->ops = data->conditions->commands[vm->conditions_pos].ops;
    vm->state = VS_IN_CONDITIONS;
    ++vm->conditions_pos;
    return;

if_statements:
    debug("setting if statements");
    vm->buffer = data->if_statements->commands[vm->if_statment_pos].vals;
    vm->buffer_lens = data->if_statements->commands[vm->if_statment_pos].lens;
    vm->ops = data->if_statements->commands[vm->if_statment_pos].ops;
    vm->state = VS_IN_IF_STATEMENTS;
    ++vm->if_statment_pos;
    return;

else_statements:
    if (data->logic_type != LT_IF_ELSE) {
        vm->tokens_end = true;
        vm->buffer[0] = NULL;
        return;
    }
    debug("setting else statements");
    vm->buffer = data->else_statements->commands[vm->else_statment_pos].vals;
    vm->buffer_lens = data->else_statements->commands[vm->else_statment_pos].lens;
    vm->ops = data->else_statements->commands[vm->else_statment_pos].ops;
    vm->state = VS_IN_ELSE_STATEMENTS;
    ++vm->else_statment_pos;
    return;

end:
    vm->tokens_end = true;
    vm->buffer[0] = NULL;
}

Token* vm_buffer_set(Token* rst tok, Token_Data* rst data, Vm_Data* rst vm)
{
    switch (data->logic_type) {
    case LT_IF:
    case LT_IF_ELSE: {
        vm_buffer_set_if(data, vm);
        return NULL;
    }
    default: {
        return vm_buffer_set_command_next(tok, vm);
    }
    }
}
