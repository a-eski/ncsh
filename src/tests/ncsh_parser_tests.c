#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_test.h"
#include "../ncsh_parser.h"

void ncsh_parser_parse_ls_test(void)
{
    char *line = "ls\0";
    uint_fast8_t length = 3;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 1);

    eskilib_assert(eskilib_string_equals(args.values[0], line, length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == length);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_ls_dash_l_test(void)
{
    char *line = "ls -l\0";
    uint_fast8_t length = 6;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], "-l", length));
    eskilib_assert(args.ops[1] == OP_CONSTANT);
    eskilib_assert(args.lengths[1] == 3);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_pipe_test(void)
{
    char *line = "ls | sort\0";
    uint_fast8_t length = 10;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], "|", length));
    eskilib_assert(args.ops[1] == OP_PIPE);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(eskilib_string_equals(args.values[2], "sort", length));
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 5);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_multiple_pipe_test(void)
{
    char *line = "ls | sort | table";
    uint_fast8_t length = 18;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 5);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], "|", length));
    eskilib_assert(args.ops[1] == OP_PIPE);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(eskilib_string_equals(args.values[2], "sort", length));
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 5);

    eskilib_assert(eskilib_string_equals(args.values[3], "|", length));
    eskilib_assert(args.ops[3] == OP_PIPE);
    eskilib_assert(args.lengths[3] == 2);

    eskilib_assert(eskilib_string_equals(args.values[4], "table", length));
    eskilib_assert(args.ops[4] == OP_CONSTANT);
    eskilib_assert(args.lengths[4] == 6);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_background_job_test(void)
{
    char *line = "longrunningprogram &\0";
    uint_fast8_t length = 21;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(eskilib_string_equals(args.values[0], "longrunningprogram", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 19);

    eskilib_assert(eskilib_string_equals(args.values[1], "&", length));
    eskilib_assert(args.ops[1] == OP_BACKGROUND_JOB);
    eskilib_assert(args.lengths[1] == 2);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_output_redirection_test(void)
{
    char *line = "ls > text.txt\0";
    uint_fast8_t length = 14;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], ">", length));
    eskilib_assert(args.ops[1] == OP_STDOUT_REDIRECTION);
    eskilib_assert(args.lengths[1] == 2);

    eskilib_assert(eskilib_string_equals(args.values[2], "text.txt", length));
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 9);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_output_redirection_append_test(void)
{
    char *line = "ls >> text.txt\0";
    uint_fast8_t length = 15;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 3);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.ops[0] == OP_CONSTANT);
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], ">>", length));
    eskilib_assert(args.ops[1] == OP_STDOUT_REDIRECTION_APPEND);
    eskilib_assert(args.lengths[1] == 3);

    eskilib_assert(eskilib_string_equals(args.values[2], "text.txt", length));
    eskilib_assert(args.ops[2] == OP_CONSTANT);
    eskilib_assert(args.lengths[2] == 9);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_double_quotes_test(void)
{
    char *line = "echo \"hello\"\0";
    uint_fast8_t length = 13;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(eskilib_string_equals(args.values[0], "echo", length));
    eskilib_assert(args.lengths[0] == 5);

    eskilib_assert(eskilib_string_equals(args.values[1], "hello", length));
    eskilib_assert(args.lengths[1] == 6);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_home_test(void)
{
    char *line = "ls ~\0";
    uint_fast8_t length = 5;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], "/home/alex", length));
    eskilib_assert(args.lengths[1] == 11);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_home_at_start_test(void)
{
    char *line = "ls ~/snap\0";
    uint_fast8_t length = 10;

    struct ncsh_Args args;
    bool result = ncsh_parser_args_malloc(&args);
    eskilib_assert(result == true);
    ncsh_parser_parse(line, length, &args);

    eskilib_assert(args.values != NULL);
    eskilib_assert(args.count == 2);

    eskilib_assert(eskilib_string_equals(args.values[0], "ls", length));
    eskilib_assert(args.lengths[0] == 3);

    eskilib_assert(eskilib_string_equals(args.values[1], "/home/alex/snap", length));
    eskilib_assert(args.lengths[1] == 16);

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
}

void ncsh_parser_parse_tests(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_parser_parse_ls_test", ncsh_parser_parse_ls_test);
    eskilib_test_run("ncsh_parser_parse_ls_dash_l_test", ncsh_parser_parse_ls_dash_l_test);
    eskilib_test_run("ncsh_parser_parse_pipe_test", ncsh_parser_parse_pipe_test);
    eskilib_test_run("ncsh_parser_parse_multiple_pipe_test", ncsh_parser_parse_multiple_pipe_test);
    eskilib_test_run("ncsh_parser_parse_background_job_test", ncsh_parser_parse_background_job_test);
    eskilib_test_run("ncsh_parser_parse_output_redirection_test", ncsh_parser_parse_output_redirection_test);
    eskilib_test_run("ncsh_parser_parse_double_quotes_test", ncsh_parser_parse_double_quotes_test);
    eskilib_test_run("ncsh_parser_parse_output_redirection_append_test",
                     ncsh_parser_parse_output_redirection_append_test);
#ifdef NDEBUG
    eskilib_test_run("ncsh_parser_parse_home_test", ncsh_parser_parse_home_test);
    eskilib_test_run("ncsh_parser_parse_home_at_start_test", ncsh_parser_parse_home_at_start_test);
#endif /* ifdef NDEBUG */

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_parser_parse_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
