#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/eskilib_test.h"
#include "../src/ncsh_parser.h"

void ncsh_parser_parse_ls_test(void)
{
    char* line = "ls\0";
    size_t length = 3;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 1);

    eskilib_assert(memcmp(args.values[0], line, length) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == length);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_ls_dash_l_test(void)
{
    char* line = "ls -l\0";
    size_t length = 6;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "-l", 3) == 0);
    eskilib_assert(args.ops[1] == OP_CONSTANT);
    eskilib_assert(args.lengths[1] == 3);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_pipe_test(void)
{
    char* line = "ls | sort\0";
    size_t length = 10;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_multiple_pipe_test(void)
{
    char* line = "ls | sort | table";
    size_t length = 18;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_background_job_test(void)
{
    char* line = "longrunningprogram &\0";
    size_t length = 21;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "longrunningprogram", 19) == 0);
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 19);

    eskilib_assert(memcmp(args.values[1], "&", 2) == 0);
    eskilib_assert(args.ops[1] == OP_BACKGROUND_JOB);
    eskilib_assert(args.lengths[1] == 2);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_output_redirection_test(void)
{
    char* line = "ls > text.txt\0";
    size_t length = 14;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_output_redirection_append_test(void)
{
    char* line = "ls >> text.txt\0";
    size_t length = 15;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_double_quotes_test(void)
{
    char* line = "echo \"hello\"\0";
    size_t length = 13;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_single_quotes_test(void)
{
    char* line = "echo \'hello\'\0";
    size_t length = 13;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_backtick_quotes_test(void)
{
    char* line = "echo `hello`\0";
    size_t length = 13;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "echo", 5) == 0);
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(memcmp(args.values[1], "hello", 6) == 0);
    eskilib_assert(args.lengths[1] == 6);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_git_commit_test(void)
{
    char* line = "git commit -m \"this is a commit message\"\0";
    size_t length = strlen(line) + 1;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_home_test(void)
{
    char* line = "ls ~\0";
    size_t length = 5;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "/home/alex", 11) == 0);
    eskilib_assert(args.lengths[1] == 11);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_home_at_start_test(void)
{
    char* line = "ls ~/snap\0";
    size_t length = 10;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(memcmp(args.values[0], "ls", 3) == 0);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(memcmp(args.values[1], "/home/alex/snap", 16) == 0);
    eskilib_assert(args.lengths[1] == 16);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_math_operators(void)
{
    char* line = "( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t length = strlen(line) + 1;
    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

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

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_glob_star_shouldnt_crash(void)
{
    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t length = strlen(line) + 1;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.count == 0);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_tilde_home_shouldnt_crash(void)
{
    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t length = strlen(line) + 1;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.count == 0);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_glob_question_and_tilde_home_shouldnt_crash(void)
{
    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t length = strlen(line) + 1;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.count == 0);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void ncsh_parser_parse_bad_input_shouldnt_crash(void);

#ifdef NDEBUG
void ncsh_parser_release_tests(void)
{
    eskilib_test_run("ncsh_parser_parse_home_test", ncsh_parser_parse_home_test);
    eskilib_test_run("ncsh_parser_parse_home_at_start_test", ncsh_parser_parse_home_at_start_test);
}
#endif /* ifdef NDEBUG */

void ncsh_parser_parse_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(ncsh_parser_parse_ls_test);
    eskilib_test_run(ncsh_parser_parse_ls_dash_l_test);
    eskilib_test_run(ncsh_parser_parse_pipe_test);
    eskilib_test_run(ncsh_parser_parse_multiple_pipe_test);
    eskilib_test_run(ncsh_parser_parse_background_job_test);
    eskilib_test_run(ncsh_parser_parse_output_redirection_test);
    eskilib_test_run(ncsh_parser_parse_double_quotes_test);
    eskilib_test_run(ncsh_parser_parse_single_quotes_test);
    eskilib_test_run(ncsh_parser_parse_backtick_quotes_test);
    eskilib_test_run(ncsh_parser_parse_output_redirection_append_test);
    eskilib_test_run(ncsh_parser_parse_git_commit_test);

#ifdef NDEBUG
    eskilib_test_run("ncsh_parser_parse_home_test", ncsh_parser_parse_home_test);
    eskilib_test_run("ncsh_parser_parse_home_at_start_test", ncsh_parser_parse_home_at_start_test);
#endif /* ifdef NDEBUG */

    eskilib_test_run(ncsh_parser_parse_math_operators);

    eskilib_test_run(ncsh_parser_parse_glob_star_shouldnt_crash);
    eskilib_test_run(ncsh_parser_parse_tilde_home_shouldnt_crash);
    eskilib_test_run(ncsh_parser_parse_glob_question_and_tilde_home_shouldnt_crash);
    eskilib_test_run(ncsh_parser_parse_bad_input_shouldnt_crash);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_parser_parse_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */

// put at the end because it messes with clangd lsp
void ncsh_parser_parse_bad_input_shouldnt_crash(void)
{
    char* line = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >";
    size_t length = strlen(line) + 1;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    // eskilib_assert(args.count == 0);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}
