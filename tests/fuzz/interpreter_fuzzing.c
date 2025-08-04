#include <setjmp.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../src/interpreter/interpreter.h"
#include "../lib/arena_test_helper.h"

jmp_buf env;
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
    // if (config_init(&shell.config, &shell.arena, shell.scratch_arena) != E_SUCCESS) {
    //     return NULL;
    // }
    //
    // if (io_init(&shell.config, &shell.input, &shell.arena) != EXIT_SUCCESS) {
    //     return NULL;
    // }
    //
    // enum z_Result z_result = z_init(&shell.config.config_location, &shell.z_db, &shell.arena);
    // if (z_result != Z_SUCCESS) {
    //     return NULL;
    // }

    interpreter_init(&shell);

    (void)interpreter_run(&shell, scratch_arena);

    SCRATCH_ARENA_TEST_TEARDOWN;
    return EXIT_SUCCESS;
}
