#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../etest.h"
#include "../../src/interpreter/lexemes.h"
#include "../../src/interpreter/lexer.h"
#include "../../src/interpreter/ops.h"
#include "../lib/arena_test_helper.h"

void lexer_lex_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t len = 3;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    eassert(lexemes.vals[0]);
    eassert(!memcmp(lexemes.vals[0], line, len));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == len);

    eassert(!lexemes.vals[1]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l\0";
    size_t len = 6;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "-l", 3));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 3);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort\0";
    size_t len = 10;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(memcmp(lexemes.vals[0], "ls", 3) == 0);
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(memcmp(lexemes.vals[1], "|", 2) == 0);
    eassert(lexemes.ops[1] == OP_PIPE);
    eassert(lexemes.lens[1] == 2);

    eassert(memcmp(lexemes.vals[2], "sort", 5) == 0);
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 5);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_multiple_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t len = 18;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "|", 2));
    eassert(lexemes.ops[1] == OP_PIPE);
    eassert(lexemes.lens[1] == 2);

    eassert(!memcmp(lexemes.vals[2], "sort", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 5);

    eassert(!memcmp(lexemes.vals[3], "|", 2));
    eassert(lexemes.ops[3] == OP_PIPE);
    eassert(lexemes.lens[3] == 2);

    eassert(!memcmp(lexemes.vals[4], "table", 6));
    eassert(lexemes.ops[4] == OP_CONSTANT);
    eassert(lexemes.lens[4] == 6);

    eassert(!lexemes.vals[5]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &\0";
    size_t len = 21;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "longrunningprogram", 19));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 19);

    eassert(!memcmp(lexemes.vals[1], "&", 2));
    eassert(lexemes.ops[1] == OP_BACKGROUND_JOB);
    eassert(lexemes.lens[1] == 2);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt\0";
    size_t len = 14;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], ">", 2));
    eassert(lexemes.ops[1] == OP_STDOUT_REDIRECTION);
    eassert(lexemes.lens[1] == 2);

    eassert(!memcmp(lexemes.vals[2], "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 9);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt\0";
    size_t len = 15;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], ">>", 3));
    eassert(lexemes.ops[1] == OP_STDOUT_REDIRECTION_APPEND);
    eassert(lexemes.lens[1] == 3);

    eassert(!memcmp(lexemes.vals[2], "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 9);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "t.txt < sort";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "t.txt", 6));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 6);

    eassert(!memcmp(lexemes.vals[1], "<", 2));
    eassert(lexemes.ops[1] == OP_STDIN_REDIRECTION);
    eassert(lexemes.lens[1] == 2);

    eassert(!memcmp(lexemes.vals[2], "sort", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 5);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt\0";
    size_t len = 15;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "&>", 3));
    eassert(lexemes.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION);
    eassert(lexemes.lens[1] == 3);

    eassert(!memcmp(lexemes.vals[2], "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 9);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "&>>", 4));
    eassert(lexemes.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION_APPEND);
    eassert(lexemes.lens[1] == 4);

    eassert(!memcmp(lexemes.vals[2], "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 9);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    size_t stripped_len = sizeof("STR=Hello");
    eassert(!memcmp(lexemes.vals[0], "STR=Hello", stripped_len));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.lens[0] == stripped_len);

    eassert(!lexemes.vals[1]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls -a\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    eassert(!memcmp(lexemes.vals[0], "STR=ls -a", len - 2));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.lens[0] == len - 2);

    eassert(!lexemes.vals[1]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls | sort\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    eassert(!memcmp(lexemes.vals[0], "STR=ls | sort", len - 2));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.lens[0] == len - 2);

    eassert(!lexemes.vals[1])

        SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo $STR";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] = 5);

    eassert(!memcmp(lexemes.vals[1], "$STR", 5));
    eassert(lexemes.ops[1] == OP_VARIABLE);
    eassert(lexemes.lens[1] = 5);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && echo $STR";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.vals[0], "STR=hello", sizeof("STR=hello")));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.lens[0] == sizeof("STR=hello"));

    eassert(!memcmp(lexemes.vals[1], "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.lens[1] == 3);

    eassert(!memcmp(lexemes.vals[2], "echo", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] = 5);

    eassert(!memcmp(lexemes.vals[3], "$STR", 5));
    eassert(lexemes.ops[3] == OP_VARIABLE);
    eassert(lexemes.lens[3] = 5);

    eassert(!lexemes.vals[4]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls && $COMMAND";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.vals[0], "COMMAND=ls", sizeof("COMMAND=ls")));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.lens[0] == sizeof("COMMAND=ls"));

    eassert(!memcmp(lexemes.vals[1], "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.lens[1] == 3);

    eassert(!memcmp(lexemes.vals[2], "$COMMAND", 8));
    eassert(lexemes.ops[2] == OP_VARIABLE);
    eassert(lexemes.lens[2] = 8);

    eassert(!lexemes.vals[3]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\"\0";
    size_t len = 13;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 5);

    eassert(!memcmp(lexemes.vals[1], "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 6);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \'hello\'\0";
    size_t len = 13;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 5);

    eassert(!memcmp(lexemes.vals[1], "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 6);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo `hello`\0";
    size_t len = 13;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 5);

    eassert(!memcmp(lexemes.vals[1], "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 6);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"\0";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.vals[0], "git", 4));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 4);

    eassert(!memcmp(lexemes.vals[1], "commit", 7));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 7);

    eassert(!memcmp(lexemes.vals[2], "-m", 3));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.lens[2] == 3);

    eassert(!memcmp(lexemes.vals[3], "this is a commit message", 25));
    eassert(lexemes.ops[3] == OP_CONSTANT);
    eassert(lexemes.lens[3] == 25);

    eassert(!lexemes.vals[4]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~\0";
    size_t len = 5;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "~", 2));
    eassert(lexemes.ops[1] == OP_HOME_EXPANSION);
    eassert(lexemes.lens[1] == 2);

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap";
    size_t len = 10;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "~/snap", sizeof("~/snap") - 1));
    eassert(lexemes.ops[1] == OP_HOME_EXPANSION);
    eassert(lexemes.lens[1] == sizeof("~/snap"));

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.ops[0] == OP_MATH_EXPRESSION_START);

    eassert(!memcmp(lexemes.vals[1], "1", 1));
    eassert(lexemes.ops[1] == OP_CONSTANT);

    eassert(lexemes.ops[2] == OP_ADD);

    eassert(!memcmp(lexemes.vals[3], "1", 1));
    eassert(lexemes.ops[3] == OP_CONSTANT);

    eassert(lexemes.ops[4] == OP_SUBTRACT);

    eassert(!memcmp(lexemes.vals[5], "1", 1));
    eassert(lexemes.ops[5] == OP_CONSTANT);

    eassert(lexemes.ops[6] == OP_MULTIPLY);

    eassert(!memcmp(lexemes.vals[7], "1", 1));
    eassert(lexemes.ops[7] == OP_CONSTANT);

    eassert(lexemes.ops[8] == OP_DIVIDE);

    eassert(!memcmp(lexemes.vals[9], "1", 1));
    eassert(lexemes.ops[9] == OP_CONSTANT);

    eassert(lexemes.ops[10] == OP_MODULO);

    eassert(!memcmp(lexemes.vals[11], "1", 1));
    eassert(lexemes.ops[11] == OP_CONSTANT);

    eassert(lexemes.ops[12] == OP_EXPONENTIATION);

    eassert(!memcmp(lexemes.vals[13], "1", 1));
    eassert(lexemes.ops[13] == OP_CONSTANT);

    eassert(lexemes.ops[14] == OP_MATH_EXPRESSION_END);

    eassert(!lexemes.vals[15]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls *.md";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "*.md", sizeof("*.md") - 1));
    eassert(lexemes.ops[1] == OP_GLOB_EXPANSION);
    eassert(lexemes.lens[1] == sizeof("*.md"));

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ?.md";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 3);

    eassert(!memcmp(lexemes.vals[1], "?.md", sizeof("?.md") - 1));
    eassert(lexemes.ops[1] == OP_GLOB_EXPANSION);
    eassert(lexemes.lens[1] == sizeof("?.md"));

    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 18);
    for (size_t i = 0; i < lexemes.count; ++i) {
        eassert(lexemes.vals[i][0] == '*');
        eassert(lexemes.ops[0] == OP_GLOB_EXPANSION);
        eassert(lexemes.lens[0] == 2);
    }

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "false && true || false";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.vals[0], "false", 6));
    eassert(lexemes.ops[0] == OP_FALSE);
    eassert(lexemes.lens[0] == 6);

    eassert(!memcmp(lexemes.vals[1], "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.lens[1] == 3);

    eassert(!memcmp(lexemes.vals[2], "true", 5));
    eassert(lexemes.ops[2] == OP_TRUE);
    eassert(lexemes.lens[2] == 5);

    eassert(!memcmp(lexemes.vals[3], "||", 3));
    eassert(lexemes.ops[3] == OP_OR);
    eassert(lexemes.lens[3] == 3);

    eassert(!memcmp(lexemes.vals[4], "false", 6));
    eassert(lexemes.ops[4] == OP_FALSE);
    eassert(lexemes.lens[4] == 6);

    eassert(!lexemes.vals[5]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 10);

    eassert(lexemes.ops[0] == OP_IF);
    eassert(lexemes.ops[1] == OP_CONDITION_START);
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.ops[3] == OP_EQUALS);
    eassert(lexemes.ops[4] == OP_CONSTANT);
    eassert(lexemes.ops[5] == OP_CONDITION_END);
    eassert(lexemes.ops[6] == OP_THEN);
    eassert(lexemes.ops[7] == OP_CONSTANT);
    eassert(lexemes.ops[8] == OP_CONSTANT);
    eassert(lexemes.ops[9] == OP_FI);
    eassert(!lexemes.ops[10])

        SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 13);

    eassert(lexemes.ops[0] == OP_IF);
    eassert(lexemes.ops[1] == OP_CONDITION_START);
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.ops[3] == OP_EQUALS);
    eassert(lexemes.ops[4] == OP_CONSTANT);
    eassert(lexemes.ops[5] == OP_CONDITION_END);
    eassert(lexemes.ops[6] == OP_THEN);
    eassert(lexemes.ops[7] == OP_CONSTANT);
    eassert(lexemes.ops[8] == OP_CONSTANT);
    eassert(lexemes.ops[9] == OP_ELSE);
    eassert(lexemes.ops[10] == OP_CONSTANT);
    eassert(lexemes.ops[11] == OP_CONSTANT);
    eassert(lexemes.ops[12] == OP_FI);
    eassert(!lexemes.ops[13])

        SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\" # this is a comment\0";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.vals[0], "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.lens[0] == 5);

    eassert(!memcmp(lexemes.vals[1], "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.lens[1] == 6);

    // comment is stripped by the lexer
    eassert(!lexemes.vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void lexer_lex_bad_input_shouldnt_crash();

void lexer_tests()
{
    etest_start();

    etest_run(lexer_lex_ls_test);
    etest_run(lexer_lex_ls_dash_l_test);
    etest_run(lexer_lex_pipe_test);
    etest_run(lexer_lex_multiple_pipe_test);
    etest_run(lexer_lex_background_job_test);
    etest_run(lexer_lex_output_redirection_test);
    etest_run(lexer_lex_double_quotes_test);
    etest_run(lexer_lex_single_quotes_test);
    etest_run(lexer_lex_backtick_quotes_test);
    etest_run(lexer_lex_output_redirection_append_test);
    etest_run(lexer_lex_input_redirection_test);
    etest_run(lexer_lex_stdout_and_stderr_redirection_test);
    etest_run(lexer_lex_stdout_and_stderr_redirection_append_test);
    etest_run(lexer_lex_assignment_test);
    etest_run(lexer_lex_assignment_spaces_test);
    etest_run(lexer_lex_assignment_spaces_multiple_test);
    etest_run(lexer_lex_variable_test);
    etest_run(lexer_lex_variable_and_test);
    etest_run(lexer_lex_variable_command_test);
    etest_run(lexer_lex_git_commit_test);
    etest_run(lexer_lex_home_test);
    etest_run(lexer_lex_home_at_start_test);
    etest_run(lexer_lex_math_operators_test);
    etest_run(lexer_lex_glob_star_test);
    etest_run(lexer_lex_glob_question_test);
    etest_run(lexer_lex_glob_star_shouldnt_crash);
    etest_run(lexer_lex_tilde_home_shouldnt_crash);
    etest_run(lexer_lex_glob_question_and_tilde_home_shouldnt_crash);
    etest_run(lexer_lex_bad_input_shouldnt_crash);
    etest_run(lexer_lex_bool_test);
    etest_run(lexer_lex_if_test);
    etest_run(lexer_lex_if_else_test);
    etest_run(lexer_lex_comment_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    lexer_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */

// put at the end because line messes with clangd lsp
void lexer_lex_bad_input_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);

    // not crashing is a test pass
    (void)lexemes;

    SCRATCH_ARENA_TEST_TEARDOWN;
}
