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

void vm_execute_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort";
    args.lengths[0] = sizeof("ls | sort");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_pipe_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort | wc -c";
    args.lengths[0] = sizeof("ls | sort | wc -c");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls > t.txt";
    args.lengths[0] = sizeof("ls > t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort > t.txt";
    args.lengths[0] = sizeof("ls | sort > t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_append_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls >> t.txt";
    args.lengths[0] = sizeof("ls >> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_append_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort >> t.txt";
    args.lengths[0] = sizeof("ls | sort >> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_err_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls &> t.txt";
    args.lengths[0] = sizeof("ls &> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_err_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort &> t.txt";
    args.lengths[0] = sizeof("ls | sort &> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_err_append_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls &>> t.txt";
    args.lengths[0] = sizeof("ls &>> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_err_append_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort &>> t.txt";
    args.lengths[0] = sizeof("ls | sort &>> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_and_err_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls 2> t.txt";
    args.lengths[0] = sizeof("ls 2> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_and_err_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort 2> t.txt";
    args.lengths[0] = sizeof("ls | sort 2> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_and_err_append_redirect_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls 2>> t.txt";
    args.lengths[0] = sizeof("ls 2>> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_out_and_err_append_redirect_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls | sort 2>> t.txt";
    args.lengths[0] = sizeof("ls | sort 2>> t.txt");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls && ls";
    args.lengths[0] = sizeof("ls && ls");
    args.ops[0] = OP_CONSTANT;
    struct Shell shell= {.args = args};

    int res = vm_execute(&shell, &scratch_arena);

    eassert(res == NCSH_COMMAND_SUCCESS_CONTINUE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vm_execute_or_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {0};
    parser_init(&args, &scratch_arena);
    args.count = 1;
    args.values[0] = "ls || ls";
    args.lengths[0] = sizeof("ls || ls");
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
    etest_run(vm_execute_pipe_test);
    etest_run(vm_execute_pipe_multiple_test);
    etest_run(vm_execute_out_redirect_test);
    etest_run(vm_execute_out_redirect_pipe_test);
    etest_run(vm_execute_out_append_redirect_test);
    etest_run(vm_execute_out_append_redirect_pipe_test);
    etest_run(vm_execute_err_redirect_test);
    etest_run(vm_execute_err_redirect_pipe_test);
    etest_run(vm_execute_err_append_redirect_test);
    etest_run(vm_execute_err_append_redirect_pipe_test);
    etest_run(vm_execute_out_and_err_redirect_test);
    etest_run(vm_execute_out_and_err_redirect_pipe_test);
    etest_run(vm_execute_out_and_err_append_redirect_test);
    etest_run(vm_execute_out_and_err_append_redirect_pipe_test);
    etest_run(vm_execute_and_test);
    etest_run(vm_execute_or_test);

    etest_finish();
}
