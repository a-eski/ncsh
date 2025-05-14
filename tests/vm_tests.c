#include <setjmp.h>
#include <signal.h>

#include "../src/defines.h"
#include "../src/eskilib/etest.h"
#include "../src/vm/vm.h"
#include "lib/arena_test_helper.h"

sig_atomic_t vm_child_pid;
jmp_buf env;

// use a macro so line numbers are preserved
#define vm_tester(input)                                                                                               \
    SCRATCH_ARENA_TEST_SETUP;                                                                                          \
                                                                                                                       \
    Args* args = parser_parse(input, strlen(input) + 1, &scratch_arena);                                               \
    Shell shell = {0};                                                                                                 \
                                                                                                                       \
    eassert(vm_execute(args, &shell, &scratch_arena) == NCSH_COMMAND_SUCCESS_CONTINUE);                                \
                                                                                                                       \
    SCRATCH_ARENA_TEST_TEARDOWN;

void running_vm_tests()
{
}

void vm_tests()
{
    etest_start();

    etest_run(running_vm_tests);
    etest_run_tester("simple_test", vm_tester("ls"));
    etest_run_tester("pipe_test", vm_tester("ls | sort"));
    etest_run_tester("pipe_multiple_test", vm_tester("ls | sort | wc -c"));
    etest_run_tester("out_redirect_test", vm_tester("ls > t.txt"));
    etest_run_tester("out_redirect_pipe_test", vm_tester("ls | sort > t.txt"));
    etest_run_tester("out_append_redirect_test", vm_tester("ls >> t.txt"));
    etest_run_tester("out_append_redirect_pipe_test", vm_tester("ls | sort >> t.txt"));
    etest_run_tester("err_redirect_test", vm_tester("ls &> t.txt"));
    etest_run_tester("err_redirect_pipe_test", vm_tester("ls | sort &> t.txt"));
    etest_run_tester("err_append_redirect_test", vm_tester("ls &>> t.txt"));
    etest_run_tester("err_append_redirect_pipe_test", vm_tester("ls | sort &>> t.txt"));
    etest_run_tester("out_and_err_redirect_test", vm_tester("ls 2> t.txt"));
    etest_run_tester("out_and_err_redirect_pipe_test", vm_tester("ls | sort 2> t.txt"));
    etest_run_tester("out_and_err_append_redirect_test", vm_tester("ls 2>> t.txt"))
        etest_run_tester("out_and_err_append_redirect_pipe_test", vm_tester("ls | sort 2>> t.txt"));
    etest_run_tester("in_redirect_test", vm_tester("sort < t.txt"));
    etest_run_tester("in_redirect_pipe_test", vm_tester("sort | wc -c < t.txt"));
    etest_run_tester("in_redirect_pipe_test", vm_tester("wc -c < t.txt | sort"));
    etest_run_tester("and_test", vm_tester("ls && ls"));
    etest_run_tester("or_test", vm_tester("ls || ls"));
    etest_run_tester("echo_test", vm_tester("echo hello"));
    etest_run_tester("echo_single_quote_test", vm_tester("echo 'hello one'"));
    etest_run_tester("echo_double_quote_test", vm_tester("echo \"hello two\""));
    etest_run_tester("echo_backtick_quote_test", vm_tester("echo `hello three`"));
    etest_run_tester("echo_out_redirect_test", vm_tester("echo hello > t.txt"));
    etest_run_tester("echo_out_append_redirect_test", vm_tester("echo hello >> t.txt"));

    etest_finish();
}

int main()
{
    vm_tests();

    remove("t.txt");
}
