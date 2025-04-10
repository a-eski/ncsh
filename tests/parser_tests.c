#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/eskilib_test.h"
#include "../src/parser.h"
#include "lib/arena_test_helper.h"

void parser_parse_ls_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t length = 3;

    struct Args args;
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 1);

    eskilib_assert(!memcmp(args.values[0], line, length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == length);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(!memcmp(args.values[0], "ls", 3));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(!memcmp(args.values[1], "-l", 3));
    eskilib_assert(args.ops[1] == OP_CONSTANT);
    eskilib_assert(args.lengths[1] == 3);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "|", 2) == 0);
    eskilib_assert(args.ops[1] == OP_PIPE);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(memcmp(args.values[2], "sort", 5) == 0);
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 5);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 5);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "|", 2) == 0);
    eskilib_assert(args.ops[1] == OP_PIPE);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(memcmp(args.values[2], "sort", 5) == 0);
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 5);

    eskilib_assert(memcmp(args.values[3], "|", 2) == 0);
    eskilib_assert(args.ops[3] == OP_PIPE);
    eskilib_assert(args.lengths[3] == 2);

    eskilib_assert(memcmp(args.values[4], "table", 6) == 0);
    eskilib_assert(args.ops[4] == OP_CONSTANT);
    eskilib_assert(args.lengths[4] == 6);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "longrunningprogram", 19) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 19);

    eskilib_assert(memcmp(args.values[1], "&", 2) == 0);
    eskilib_assert(args.ops[1] == OP_BACKGROUND_JOB);
    eskilib_assert(args.lengths[1] == 2);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], ">", 2) == 0);
    eskilib_assert(args.ops[1] == OP_STDOUT_REDIRECTION);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(memcmp(args.values[2], "text.txt", 9) == 0);
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 9);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], ">>", 3) == 0);
    eskilib_assert(args.ops[1] == OP_STDOUT_REDIRECTION_APPEND);
    eskilib_assert(args.lengths[1] == 3);

    eskilib_assert(memcmp(args.values[2], "text.txt", 9) == 0);
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 9);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 4);

    eskilib_assert(memcmp(args.values[0], "git", 4) == 0);
    eskilib_assert(args.lengths[0] == 4);
    eskilib_assert(args.ops[0] == OP_CONSTANT);

    eskilib_assert(memcmp(args.values[1], "commit", 7) == 0);
    eskilib_assert(args.lengths[1] == 7);
    eskilib_assert(args.ops[1] == OP_CONSTANT);

    eskilib_assert(memcmp(args.values[2], "-m", 3) == 0);
    eskilib_assert(args.lengths[2] == 3);
    eskilib_assert(args.ops[2] == OP_CONSTANT);

    eskilib_assert(memcmp(args.values[3], "this is a commit message", 25) == 0);
    eskilib_assert(args.lengths[3] == 25);
    eskilib_assert(args.ops[3] == OP_CONSTANT);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "/home/alex", 11) == 0);
    eskilib_assert(args.lengths[1] == 11);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "/home/alex/snap", 16) == 0);
    eskilib_assert(args.lengths[1] == 16);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.ops[0] == OP_MATH_EXPRESSION_START);

    eskilib_assert(args.ops[1] == OP_CONSTANT);
    eskilib_assert(args.ops[3] == OP_CONSTANT);
    eskilib_assert(args.ops[5] == OP_CONSTANT);
    eskilib_assert(args.ops[7] == OP_CONSTANT);
    eskilib_assert(args.ops[9] == OP_CONSTANT);
    eskilib_assert(args.ops[11] == OP_CONSTANT);
    eskilib_assert(args.ops[13] == OP_CONSTANT);

    eskilib_assert(args.ops[2] == OP_ADD);
    eskilib_assert(args.ops[4] == OP_SUBTRACT);
    eskilib_assert(args.ops[6] == OP_MULTIPLY);
    eskilib_assert(args.ops[8] == OP_DIVIDE);
    eskilib_assert(args.ops[10] == OP_MODULO);
    eskilib_assert(args.ops[12] == OP_EXPONENTIATION);

    eskilib_assert(args.ops[14] == OP_MATH_EXPRESSION_END);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.count == 0);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.count == 0);

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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    eskilib_assert(args.count == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void parser_parse_bad_input_shouldnt_crash(void);

#ifdef NDEBUG
void parser_release_tests(void)
{
    eskilib_test_run("parser_parse_home_test", parser_parse_home_test);
    eskilib_test_run("parser_parse_home_at_start_test", parser_parse_home_at_start_test);
}
#endif /* ifdef NDEBUG */

void parser_parse_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(parser_parse_ls_test);
    eskilib_test_run(parser_parse_ls_dash_l_test);
    eskilib_test_run(parser_parse_pipe_test);
    eskilib_test_run(parser_parse_multiple_pipe_test);
    eskilib_test_run(parser_parse_background_job_test);
    eskilib_test_run(parser_parse_output_redirection_test);
    eskilib_test_run(parser_parse_double_quotes_test);
    eskilib_test_run(parser_parse_single_quotes_test);
    eskilib_test_run(parser_parse_backtick_quotes_test);
    eskilib_test_run(parser_parse_output_redirection_append_test);
    eskilib_test_run(parser_parse_git_commit_test);

#ifdef NDEBUG
    eskilib_test_run(parser_parse_home_test);
    eskilib_test_run(parser_parse_home_at_start_test);
#endif /* ifdef NDEBUG */

    eskilib_test_run(parser_parse_math_operators);

    eskilib_test_run(parser_parse_glob_star_shouldnt_crash);
    eskilib_test_run(parser_parse_tilde_home_shouldnt_crash);
    eskilib_test_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);
    eskilib_test_run(parser_parse_bad_input_shouldnt_crash);

    eskilib_test_finish();
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
    bool result = parser_args_alloc(&args, &arena);
    eskilib_assert(result == true);
    parser_parse(line, length, &args, &scratch_arena);

    // no crash is passing test here

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}
