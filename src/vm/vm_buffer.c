/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_buffer.c: setup commands in a form that can be sent to execvp/execve/etc. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "../args.h"
#include "vm_types.h"

Arg* vm_buffer_set_command_next(Arg* rst arg, Vm_Data* rst vm)
{
    if (arg && arg->op == OP_ASSIGNMENT) {
        arg = arg->next;
        if (arg && (arg->op == OP_AND || arg->op == OP_OR))
            arg = arg->next;
    }

    if (!arg) {
        vm->buffer[0] = NULL;
        vm->args_end = true;
        return NULL;
    }

    size_t vm_buf_pos = 0;
    while (arg && arg->val) {
        if (arg->op == OP_PIPE || arg->op == OP_AND || arg->op == OP_OR)
            break;
        assert(arg->val[arg->len - 1] == '\0');
        vm->buffer[vm_buf_pos] = arg->val;
        vm->buffer_lens[vm_buf_pos] = arg->len;
        debugf("set vm->buffer[%zu] to %s, %zu\n", vm_buf_pos, arg->val, arg->len);

        if (!arg->next || !arg->next->val) {
            vm->args_end = true;
            ++vm_buf_pos;
            break;
        }

        ++vm_buf_pos;
        arg = arg->next;
    }

    if (!vm->args_end) {
        vm->op_current = arg->op;
        debugf("set op current to %d\n", vm->op_current);
        arg = arg->next;
    }

    vm->buffer[vm_buf_pos] = NULL;

    return arg;
}

void vm_buffer_set_if(Token_Data* rst tokens, Vm_Data* rst vm)
{
    switch (vm->state) {
    case VS_NORMAL: {
        debug("setting conditions");
        vm->buffer = tokens->conditions->vals;
        vm->buffer_lens = tokens->conditions->lens;
        vm->state = VS_IN_CONDITIONS;
        return;
    }
    // conditions just processed, decide what to do next
    case VS_IN_CONDITIONS: {
        if (tokens->logic_type == LT_IF || vm->status == EXIT_SUCCESS)
            goto if_statements;
        else
            goto else_statements;
    }
    case VS_IN_IF_STATEMENTS:
    case VS_IN_ELSE_STATEMENTS: {
        vm->args_end = true;
        vm->buffer[0] = NULL;
    }
    default: {
        return;
    }
    }

if_statements:
    debug("setting if statements");
    vm->buffer = tokens->if_statements->commands[vm->if_statment_pos].vals;
    vm->buffer_lens = tokens->if_statements->commands[vm->if_statment_pos].lens;
    if (vm->if_statment_pos == tokens->if_statements->count - 1)
        vm->state = VS_IN_IF_STATEMENTS;
    ++vm->if_statment_pos;
    return;

else_statements:
    if (tokens->logic_type != LT_IF_ELSE) {
        vm->args_end = true;
        vm->buffer[0] = NULL;
        return;
    }
    debug("setting else statements");
    vm->buffer = tokens->else_statements->commands[vm->else_statment_pos].vals;
    vm->buffer_lens = tokens->else_statements->commands[vm->else_statment_pos].lens;
    if (vm->else_statment_pos == tokens->else_statements->count - 1)
        vm->state = VS_IN_ELSE_STATEMENTS;
    ++vm->else_statment_pos;
}

Arg* vm_buffer_set(Arg* rst arg, Token_Data* rst tokens, Vm_Data* rst vm)
{
    switch (tokens->logic_type) {
    case LT_IF:
    case LT_IF_ELSE: {
        vm_buffer_set_if(tokens, vm);
        return NULL;
    }
    default: {
        return vm_buffer_set_command_next(arg, vm);
    }
    }
}
