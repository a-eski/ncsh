#include <signal.h>

#include "../src/defines.h"
#include "../src/vm/vm.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

sig_atomic_t vm_child_pid;

void vm_execute_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls";
    args.lengths[0] = sizeof("ls");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(vm_execute_test);

    etest_finish();
}
