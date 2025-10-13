#pragma once

#include <setjmp.h>

#include "../../src/interpreter/parse.h"
#include "../../src/interpreter/vm_types.h"
#include "../etest.h"
#include "../lib/arena_test_helper.h"

__sig_atomic_t vm_child_pid;
jmp_buf env_jmp_buf;
volatile int sigwinch_caught;

Commands* vm_next(Commands* cmds, Vm_Data* restrict vm);

static inline void vm_setup(Vm_Data* vm, Parser_Output rv, Arena* s)
{
    // simulate setup the VM does
    vm->stmts = rv.output.stmts;
    vm->cur_stmt = rv.output.stmts->head;
    vm->s = s;
    vm->cur_cmds = vm->cur_stmt->commands;
    vm->cur_cmds->pos = 0;
}
