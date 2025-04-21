#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/etest.h"
#include "../src/parser.h"
#include "lib/arena_test_helper.h"

void parser_parse_ls_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t length = 3;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 1);

    eassert(!memcmp(args.values[0], line, length));
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == length);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_ls_dash_l_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l\0";
    size_t length = 6;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(!memcmp(args.values[0], "ls", 3));
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(!memcmp(args.values[1], "-l", 3));
    eassert(args.ops[1] == OP_CONSTANT);
    eassert(args.lengths[1] == 3);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_pipe_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort\0";
    size_t length = 10;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "|", 2) == 0);
    eassert(args.ops[1] == OP_PIPE);
    eassert(args.lengths[1] == 2);

    eassert(memcmp(args.values[2], "sort", 5) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 5);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipe_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t length = 18;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 5);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "|", 2) == 0);
    eassert(args.ops[1] == OP_PIPE);
    eassert(args.lengths[1] == 2);

    eassert(memcmp(args.values[2], "sort", 5) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 5);

    eassert(memcmp(args.values[3], "|", 2) == 0);
    eassert(args.ops[3] == OP_PIPE);
    eassert(args.lengths[3] == 2);

    eassert(memcmp(args.values[4], "table", 6) == 0);
    eassert(args.ops[4] == OP_CONSTANT);
    eassert(args.lengths[4] == 6);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &\0";
    size_t length = 21;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "longrunningprogram", 19) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 19);

    eassert(memcmp(args.values[1], "&", 2) == 0);
    eassert(args.ops[1] == OP_BACKGROUND_JOB);
    eassert(args.lengths[1] == 2);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt\0";
    size_t length = 14;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], ">", 2) == 0);
    eassert(args.ops[1] == OP_STDOUT_REDIRECTION);
    eassert(args.lengths[1] == 2);

    eassert(memcmp(args.values[2], "text.txt", 9) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 9);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_append_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt\0";
    size_t length = 15;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], ">>", 3) == 0);
    eassert(args.ops[1] == OP_STDOUT_REDIRECTION_APPEND);
    eassert(args.lengths[1] == 3);

    eassert(memcmp(args.values[2], "text.txt", 9) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 9);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "t.txt < sort";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "t.txt", 6) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 6);

    eassert(memcmp(args.values[1], "<", 2) == 0);
    eassert(args.ops[1] == OP_STDIN_REDIRECTION);
    eassert(args.lengths[1] == 2);

    eassert(memcmp(args.values[2], "sort", 5) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 5);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt\0";
    size_t length = 15;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "&>", 3) == 0);
    eassert(args.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION);
    eassert(args.lengths[1] == 3);

    eassert(memcmp(args.values[2], "text.txt", 9) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 9);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_append_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 3);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.ops[0] == OP_CONSTANT);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "&>>", 4) == 0);
    eassert(args.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION_APPEND);
    eassert(args.lengths[1] == 4);

    eassert(memcmp(args.values[2], "text.txt", 9) == 0);
    eassert(args.ops[2] == OP_CONSTANT);
    eassert(args.lengths[2] == 9);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 0);

    char* var = "STR";
    struct estr* val = var_get(var, &args.vars);
    eassert(val->value);
    eassert(!memcmp(val->value, "Hello", sizeof("Hello")));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 0);

    char* var = "$STR";
    size_t var_len = strlen(var);
    parser_parse(var, var_len + 1, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 1);
    eassert(args.ops[0] == OP_VARIABLE);
    eassert(!memcmp(args.values[0], var, args.lengths[0]));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_and_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && $STR";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 1);
    eassert(args.ops[0] == OP_VARIABLE);
    eassert(!memcmp(args.values[0], "$STR", args.lengths[0]));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_command_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 0);

    char* var = "$COMMAND";
    size_t var_len = strlen(var);
    parser_parse(var, var_len + 1, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 1);
    eassert(args.ops[0] == OP_VARIABLE);
    eassert(!memcmp(args.values[0], var, args.lengths[0]));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_double_quotes_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\"\0";
    size_t length = 13;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "echo", 5) == 0);
    eassert(args.lengths[0] == 5);

    eassert(memcmp(args.values[1], "hello", 6) == 0);
    eassert(args.lengths[1] == 6);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_single_quotes_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \'hello\'\0";
    size_t length = 13;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "echo", 5) == 0);
    eassert(args.lengths[0] == 5);

    eassert(memcmp(args.values[1], "hello", 6) == 0);
    eassert(args.lengths[1] == 6);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_backtick_quotes_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo `hello`\0";
    size_t length = 13;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "echo", 5) == 0);
    eassert(args.lengths[0] == 5);

    eassert(memcmp(args.values[1], "hello", 6) == 0);
    eassert(args.lengths[1] == 6);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_git_commit_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"\0";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 4);

    eassert(memcmp(args.values[0], "git", 4) == 0);
    eassert(args.lengths[0] == 4);
    eassert(args.ops[0] == OP_CONSTANT);

    eassert(memcmp(args.values[1], "commit", 7) == 0);
    eassert(args.lengths[1] == 7);
    eassert(args.ops[1] == OP_CONSTANT);

    eassert(memcmp(args.values[2], "-m", 3) == 0);
    eassert(args.lengths[2] == 3);
    eassert(args.ops[2] == OP_CONSTANT);

    eassert(memcmp(args.values[3], "this is a commit message", 25) == 0);
    eassert(args.lengths[3] == 25);
    eassert(args.ops[3] == OP_CONSTANT);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~\0";
    size_t length = 5;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "/home/alex", 11) == 0);
    eassert(args.lengths[1] == 11);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_at_start_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap\0";
    size_t length = 10;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.values);
    eassert(args.count == 2);

    eassert(memcmp(args.values[0], "ls", 3) == 0);
    eassert(args.lengths[0] == 3);

    eassert(memcmp(args.values[1], "/home/alex/snap", 16) == 0);
    eassert(args.lengths[1] == 16);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_math_operators(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t length = strlen(line) + 1;
    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.ops[0] == OP_MATH_EXPRESSION_START);

    eassert(args.ops[1] == OP_CONSTANT);
    eassert(args.ops[3] == OP_CONSTANT);
    eassert(args.ops[5] == OP_CONSTANT);
    eassert(args.ops[7] == OP_CONSTANT);
    eassert(args.ops[9] == OP_CONSTANT);
    eassert(args.ops[11] == OP_CONSTANT);
    eassert(args.ops[13] == OP_CONSTANT);

    eassert(args.ops[2] == OP_ADD);
    eassert(args.ops[4] == OP_SUBTRACT);
    eassert(args.ops[6] == OP_MULTIPLY);
    eassert(args.ops[8] == OP_DIVIDE);
    eassert(args.ops[10] == OP_MODULO);
    eassert(args.ops[12] == OP_EXPONENTIATION);

    eassert(args.ops[14] == OP_MATH_EXPRESSION_END);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_shouldnt_crash(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.count == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_tilde_home_shouldnt_crash(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.count == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_and_tilde_home_shouldnt_crash(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    eassert(args.count == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void parser_parse_bad_input_shouldnt_crash(void);

#ifdef NDEBUG
void parser_release_tests(void)
{
    etest_run(parser_parse_home_test);
    etest_run(parser_parse_home_at_start_test);
}
#endif /* ifdef NDEBUG */

void parser_parse_tests(void)
{
    etest_start();

    etest_run(parser_parse_ls_test);
    etest_run(parser_parse_ls_dash_l_test);
    etest_run(parser_parse_pipe_test);
    etest_run(parser_parse_multiple_pipe_test);
    etest_run(parser_parse_background_job_test);
    etest_run(parser_parse_output_redirection_test);
    etest_run(parser_parse_double_quotes_test);
    etest_run(parser_parse_single_quotes_test);
    etest_run(parser_parse_backtick_quotes_test);
    etest_run(parser_parse_output_redirection_append_test);
    etest_run(parser_parse_input_redirection_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_append_test);
    etest_run(parser_parse_assignment_test);
    etest_run(parser_parse_variable_test);
    etest_run(parser_parse_variable_and_test);
    etest_run(parser_parse_variable_command_test);
    etest_run(parser_parse_git_commit_test);

#ifdef NDEBUG
    etest_run(parser_parse_home_test);
    etest_run(parser_parse_home_at_start_test);
#endif /* ifdef NDEBUG */

    etest_run(parser_parse_math_operators);

    etest_run(parser_parse_glob_star_shouldnt_crash);
    etest_run(parser_parse_tilde_home_shouldnt_crash);
    etest_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);
    etest_run(parser_parse_bad_input_shouldnt_crash);

    etest_finish();
}

int main(void)
{
    parser_parse_tests();

    return EXIT_SUCCESS;
}

// put at the end because it messes with clangd lsp
void parser_parse_bad_input_shouldnt_crash(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >";
    size_t length = strlen(line) + 1;

    struct Args args;
    enum eresult init_res = parser_init(&args, &arena);
    eassert(init_res == E_SUCCESS);
    parser_parse(line, length, &args, &arena, &scratch_arena);

    // no crash is passing test here

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}
