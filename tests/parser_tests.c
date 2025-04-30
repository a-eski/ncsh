#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/etest.h"
#include "../src/parser.h"
#include "lib/arena_test_helper.h"

void parser_parse_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t len = 3;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args);
    eassert(args->head);
    eassert(args->count == 1);

    struct Arg* arg = args->head->next;
    eassert(arg);
    eassert(!memcmp(arg->val, line, len));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == len);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l\0";
    size_t len = 6;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args);
    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "-l", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort\0";
    size_t len = 10;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args);
    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(memcmp(arg->val, "ls", 3) == 0);
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(memcmp(arg->val, "|", 2) == 0);
    eassert(arg->op == OP_PIPE);
    eassert(arg->len == 2);
    arg = arg->next;

    eassert(memcmp(arg->val, "sort", 5) == 0);
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t len = 18;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args);
    eassert(args->head);
    eassert(args->count == 5);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "|", 2));
    eassert(arg->op == OP_PIPE);
    eassert(arg->len == 2);
    arg = arg->next;

    eassert(!memcmp(arg->val, "sort", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "|", 2));
    eassert(arg->op == OP_PIPE);
    eassert(arg->len == 2);
    arg = arg->next;

    eassert(!memcmp(arg->val, "table", 6));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 6);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &\0";
    size_t len = 21;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args);
    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "longrunningprogram", 19));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 19);
    arg = arg->next;

    eassert(!memcmp(arg->val, "&", 2));
    eassert(arg->op == OP_BACKGROUND_JOB);
    eassert(arg->len == 2);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt\0";
    size_t len = 14;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, ">", 2));
    eassert(arg->op == OP_STDOUT_REDIRECTION);
    eassert(arg->len == 2);
    arg = arg->next;

    eassert(!memcmp(arg->val, "text.txt", 9));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 9);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt\0";
    size_t len = 15;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, ">>", 3));
    eassert(arg->op == OP_STDOUT_REDIRECTION_APPEND);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "text.txt", 9));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 9);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "t.txt < sort";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "t.txt", 6));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 6);
    arg = arg->next;

    eassert(!memcmp(arg->val, "<", 2));
    eassert(arg->op == OP_STDIN_REDIRECTION);
    eassert(arg->len == 2);
    arg = arg->next;

    eassert(!memcmp(arg->val, "sort", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt\0";
    size_t len = 15;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "&>", 3));
    eassert(arg->op == OP_STDOUT_AND_STDERR_REDIRECTION);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "text.txt", 9));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 9);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "&>>", 4));
    eassert(arg->op == OP_STDOUT_AND_STDERR_REDIRECTION_APPEND);
    eassert(arg->len == 4);
    arg = arg->next;

    eassert(!memcmp(arg->val, "text.txt", 9));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 9);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 1);

    struct Arg* arg = args->head->next;
    printf("%s\n", arg->val);
    // quotes stripped by the parser
    eassert(!memcmp(arg->val, "STR=Hello", sizeof("STR=Hello")));
    eassert(arg->op == OP_ASSIGNMENT);
    eassert(arg->len == sizeof("STR=Hello"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo $STR";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "echo", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len = 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "$STR", 5));
    eassert(arg->op == OP_VARIABLE);
    eassert(arg->len = 5);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && echo $STR";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 4);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "STR=hello", sizeof("STR=hello")));
    eassert(arg->op == OP_ASSIGNMENT);
    eassert(arg->len == sizeof("STR=hello"));
    arg = arg->next;

    eassert(!memcmp(arg->val, "&&", 3));
    eassert(arg->op == OP_AND);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "echo", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len = 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "$STR", 5));
    eassert(arg->op == OP_VARIABLE);
    eassert(arg->len = 5);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls && $COMMAND";
    size_t len = strlen(line) + 1;


    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 3);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "COMMAND=ls", sizeof("COMMAND=ls")));
    eassert(arg->op == OP_ASSIGNMENT);
    eassert(arg->len == sizeof("COMMAND=ls"));
    arg = arg->next;

    eassert(!memcmp(arg->val, "&&", 3));
    eassert(arg->op == OP_AND);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "$COMMAND", 8));
    eassert(arg->op == OP_VARIABLE);
    eassert(arg->len = 8);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\"\0";
    size_t len = 13;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "echo", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "hello", 6));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 6);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \'hello\'\0";
    size_t len = 13;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "echo", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "hello", 6));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 6);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo `hello`\0";
    size_t len = 13;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "echo", 5));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "hello", 6));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 6);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"\0";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 4);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "git", 4));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 4);
    arg = arg->next;

    eassert(!memcmp(arg->val, "commit", 7));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 7);
    arg = arg->next;

    eassert(!memcmp(arg->val, "-m", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "this is a commit message", 25));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 25);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~\0";
    size_t len = 5;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "~", 2));
    eassert(arg->op == OP_HOME_EXPANSION);
    eassert(arg->len == 2);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap";
    size_t len = 10;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "~/snap", sizeof("~/snap") - 1));
    eassert(arg->op == OP_HOME_EXPANSION);
    eassert(arg->len == sizeof("~/snap"));

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t len = strlen(line) + 1;
    struct Args* args = parser_parse(line, len, &scratch_arena);

    struct Arg* arg = args->head->next;
    eassert(arg->op == OP_MATH_EXPRESSION_START);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_ADD);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_SUBTRACT);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_MULTIPLY);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_DIVIDE);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_MODULO);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_EXPONENTIATION);
    arg = arg->next;

    eassert(!memcmp(arg->val, "1", 1));
    eassert(arg->op == OP_CONSTANT);
    arg = arg->next;

    eassert(arg->op == OP_MATH_EXPRESSION_END);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls *.md";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "*.md", sizeof("*.md") - 1));
    eassert(arg->op == OP_GLOB_EXPANSION);
    eassert(arg->len == sizeof("*.md"));

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ?.md";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 2);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "ls", 3));
    eassert(arg->op == OP_CONSTANT);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "?.md", sizeof("?.md") - 1));
    eassert(arg->op == OP_GLOB_EXPANSION);
    eassert(arg->len == sizeof("?.md"));

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->count == 18);
    struct Arg* arg = args->head->next;
    for (size_t i = 0; i < args->count; ++i) {
        eassert(arg->val[0] == '*');
        eassert(arg->op == OP_GLOB_EXPANSION);
        eassert(arg->len == 2);
        arg = arg->next;
    }

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "false && true || false";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    eassert(args->head);
    eassert(args->count == 5);

    struct Arg* arg = args->head->next;
    eassert(!memcmp(arg->val, "false", 6));
    eassert(arg->op == OP_FALSE);
    eassert(arg->len == 6);
    arg = arg->next;

    eassert(!memcmp(arg->val, "&&", 3));
    eassert(arg->op == OP_AND);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "true", 5));
    eassert(arg->op == OP_TRUE);
    eassert(arg->len == 5);
    arg = arg->next;

    eassert(!memcmp(arg->val, "||", 3));
    eassert(arg->op == OP_OR);
    eassert(arg->len == 3);
    arg = arg->next;

    eassert(!memcmp(arg->val, "false", 6));
    eassert(arg->op == OP_FALSE);
    eassert(arg->len == 6);

    eassert(!arg->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void parser_parse_bad_input_shouldnt_crash();

int main()
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
    etest_run(parser_parse_home_test);
    etest_run(parser_parse_home_at_start_test);
    etest_run(parser_parse_math_operators_test);
    etest_run(parser_parse_glob_star_test);
    etest_run(parser_parse_glob_question_test);
    etest_run(parser_parse_glob_star_shouldnt_crash);
    etest_run(parser_parse_tilde_home_shouldnt_crash);
    etest_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);
    etest_run(parser_parse_bad_input_shouldnt_crash);
    etest_run(parser_parse_bool_test);

    etest_finish();

    return EXIT_SUCCESS;
}

// put at the end because it messes with clangd lsp
void parser_parse_bad_input_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >";
    size_t len = strlen(line) + 1;

    struct Args* args = parser_parse(line, len, &scratch_arena);

    // hits limit so does not process, not crashing is a test pass
    (void)args;

    SCRATCH_ARENA_TEST_TEARDOWN;
}
