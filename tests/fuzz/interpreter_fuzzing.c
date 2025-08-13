#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../src/interpreter/interpreter.h"
#include "../lib/arena_test_helper.h"

jmp_buf env_jmp_buf;
sig_atomic_t vm_child_pid;
volatile int sigwinch_caught;

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell.arena = scratch_arena;
    shell.scratch_arena = scratch_arena;
    shell.input.buffer = (char*)Data;
    shell.input.pos = Size;
    interpreter_init(&shell);

    (void)interpreter_run(&shell, scratch_arena);

    SCRATCH_ARENA_TEST_TEARDOWN;
    return EXIT_SUCCESS;
}
