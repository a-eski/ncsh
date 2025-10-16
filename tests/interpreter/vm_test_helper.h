#pragma once

#include <setjmp.h>

#include "../../src/interpreter/parse.h"
#include "../../src/interpreter/vm_types.h"
#include "../etest.h"
#include "../lib/arena_test_helper.h"

__sig_atomic_t vm_child_pid;
jmp_buf env_jmp_buf;
volatile int sigwinch_caught;

Commands* vm_next(Vm_Data* restrict vm);

static inline void vm_setup(Vm_Data* vm, Parser_Output rv, Arena* s)
{
    *vm = (Vm_Data){.stmts = rv.output.stmts, .cur_stmt = rv.output.stmts->head, .sh = NULL, .s = s};
    vm->next_cmds = vm->cur_stmt->commands;
    vm->next_cmds->pos = 0;
    // simulate setup the VM does
    // vm->stmts = rv.output.stmts;
    // vm->cur_stmt = rv.output.stmts->head;
    // vm->s = s;
    // vm->next_cmds = vm->cur_stmt->commands;
    // vm->next_cmds->pos = 0;
}
