#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>

#include "../../../src/defines.h"
#include "../../../src/eskilib/etest.h"
#include "../../../src/interpreter/lexer.h"
#include "../../../src/interpreter/parser.h"
#include "../../../src/interpreter/semantic_analyzer.h"
#include "../../../src/interpreter/vm/vm.h"
#include "../../lib/arena_test_helper.h"

sig_atomic_t vm_child_pid;
jmp_buf env;

// use a macro so line numbers are preserved
#define vm_tester(input)                                                                                               \
    SCRATCH_ARENA_TEST_SETUP;                                                                                          \
                                                                                                                       \
    Lexemes lexemes = {0};                                                                                             \
    lexer_lex(input, strlen(input) + 1, &lexemes, &scratch_arena);                                                     \
    int res = semantic_analyzer_analyze(&lexemes);                                                                     \
    eassert(res == EXIT_SUCCESS);                                                                                      \
    Statements statements = {0};                                                                                       \
    Shell shell = {0};                                                                                                 \
    res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);                                                   \
    eassert(res == EXIT_SUCCESS);                                                                                      \
    res = vm_execute(&statements, &shell, &scratch_arena);                                                             \
    eassert(res == EXIT_SUCCESS || res == EXIT_FAILURE_CONTINUE);                                                      \
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
    etest_run_tester("out_and_err_append_redirect_test", vm_tester("ls 2>> t.txt"));
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
    // TODO: figure out why this causes OOM in tests
    /* etest_run_tester("if_true_test", vm_tester("if [ true ]; then echo hello; fi"));
    etest_run_tester("if_false_test", vm_tester("if [ false ]; then echo hello; fi"));
    etest_run_tester("if_else_true_test", vm_tester("if [ true ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_else_false_test", vm_tester("if [ false ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_equals_test", vm_tester("if [ 1 -eq 1 ]; then echo hello; fi"));
    etest_run_tester("if_not_equals_test", vm_tester("if [ 2 -eq 1 ]; then echo hello; fi"));
    etest_run_tester("if_else_equals_test", vm_tester("if [ 1 -eq 1 ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_else_not_equals_test", vm_tester("if [ 1 -eq 2 ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_gt_test", vm_tester("if [ 2 -gt 1 ]; then echo hello; fi"));
    etest_run_tester("if_not_gt_test", vm_tester("if [ 1 -gt 2 ]; then echo hello; fi"));
    etest_run_tester("if_else_gt_test", vm_tester("if [ 2 -gt 1 ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_else_not_gt_test", vm_tester("if [ 1 -gt 2 ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_lt_test", vm_tester("if [ 1 -lt 2 ]; then echo hello; fi"));
    etest_run_tester("if_not_lt_test", vm_tester("if [ 1 -lt 2 ]; then echo hello; fi"));
    etest_run_tester("if_else_lt_test", vm_tester("if [ 1 -lt 2 ]; then echo hello; else echo hi; fi"));
    etest_run_tester("if_else_not_lt_test", vm_tester("if [ 2 -lt 1 ]; then echo hello; else echo hi; fi"));*/

    etest_finish();
}

int main()
{
    vm_tests();

    remove("t.txt");
}
