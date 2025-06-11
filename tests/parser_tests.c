#include <unistd.h>

#include "../src/eskilib/etest.h"
#include "../src/interpreter/interpreter_types.h"
#include "../src/interpreter/lexer.h"
#include "../src/interpreter/parser.h"
#include "../src/types.h"
#include "lib/arena_test_helper.h"

void parser_parse_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t len = 3;
    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l\0";
    size_t len = 6;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort\0";
    size_t len = 10;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    // TODO: fix this being 2, should be 1
    eassert(toks->data.number_of_pipe_commands == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t len = 18;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    // TODO: fix this being 3, should be 2
    eassert(toks->data.number_of_pipe_commands == 3);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &\0";
    size_t len = 21;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.is_background_job);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt\0";
    size_t len = 14;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stdout_file);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt\0";
    size_t len = 15;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stdout_file);
    eassert(toks->data.output_append);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "t.txt < sort";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stdin_file);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls 2> text.txt\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stderr_file);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls 2>> text.txt\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stderr_file);
    eassert(toks->data.output_append);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt\0";
    size_t len = 15;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stdout_and_stderr_file);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.stdout_and_stderr_file);
    eassert(toks->data.output_append);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls -a\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls | sort\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);
    eassert(toks->data.number_of_pipe_commands == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo $STR";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && echo $STR";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls && $COMMAND";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\"\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \'hello\'\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo `hello`\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 4);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~\0";
    size_t len = 5;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap";
    size_t len = 10;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t len = strlen(line) + 1;
    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls *.md";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ?.md";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    Shell shell = {0};
    int res = parser_parse(toks, &shell, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "false && true || false";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\" # this is a comment\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);
    int res = parser_parse(toks, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

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
    etest_run(parser_parse_stderr_redirection_test);
    etest_run(parser_parse_stderr_redirection_append_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_append_test);
    /*etest_run(parser_parse_assignment_test);
    etest_run(parser_parse_assignment_spaces_test);
    etest_run(parser_parse_assignment_spaces_multiple_test);
    etest_run(parser_parse_variable_test);
    etest_run(parser_parse_variable_and_test);
    etest_run(parser_parse_variable_command_test);*/
    etest_run(parser_parse_git_commit_test);
    /*etest_run(parser_parse_home_test);
    etest_run(parser_parse_home_at_start_test);*/
    etest_run(parser_parse_math_operators_test);
    etest_run(parser_parse_glob_star_test);
    etest_run(parser_parse_glob_question_test);
    etest_run(parser_parse_glob_star_shouldnt_crash);
    /*etest_run(parser_parse_tilde_home_shouldnt_crash);
    etest_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);*/
    etest_run(parser_parse_bool_test);
    etest_run(parser_parse_if_test);
    etest_run(parser_parse_if_else_test);
    etest_run(parser_parse_comment_test);

    etest_finish();

    return EXIT_SUCCESS;
}
