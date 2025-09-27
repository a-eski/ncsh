#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../etest.h"
#include "../../src/interpreter/lex.h"
#include "../../src/interpreter/ops.h"
#include "../lib/arena_test_helper.h"

[[maybe_unused]]
void lex_print(Lexemes* lexemes)
{
    printf("count %zu\n", lexemes->count);

    for (size_t i = 0; i < lexemes->count; ++i)
        printf("%s, %zu\n", lexemes->strs[i].value, i);
}

void lex_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    eassert(lexemes.strs[0].value);
    eassert(!memcmp(lexemes.strs[0].value, line.value, line.length));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == line.length);

    eassert(!lexemes.strs[1].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls -l");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "-l", 3));
    eassert(lexemes.ops[1] == T_CONST);
    eassert(lexemes.strs[1].length == 3);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(memcmp(lexemes.strs[0].value, "ls", 3) == 0);
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(memcmp(lexemes.strs[1].value, "|", 2) == 0);
    eassert(lexemes.ops[1] == T_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(memcmp(lexemes.strs[2].value, "sort", 5) == 0);
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 5);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_multiple_pipes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort | table");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "|", 2));
    eassert(lexemes.ops[1] == T_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 5));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 5);

    eassert(!memcmp(lexemes.strs[3].value, "|", 2));
    eassert(lexemes.ops[3] == T_PIPE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!memcmp(lexemes.strs[4].value, "table", 6));
    eassert(lexemes.ops[4] == T_CONST);
    eassert(lexemes.strs[4].length == 6);

    eassert(!lexemes.strs[5].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_multiple_pipes_multiple_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort | head -1 | wc -c");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 9);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 2));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(memcmp(lexemes.strs[1].value, "|", 2) == 0);
    eassert(lexemes.ops[1] == T_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 4));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 5);

    eassert(!memcmp(lexemes.strs[3].value, "|", 2));
    eassert(lexemes.ops[3] == T_PIPE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!memcmp(lexemes.strs[4].value, "head", 4));
    eassert(lexemes.ops[4] == T_CONST);
    eassert(lexemes.strs[4].length == 5);

    eassert(!memcmp(lexemes.strs[5].value, "-1", 2));
    eassert(lexemes.ops[5] == T_CONST);
    eassert(lexemes.strs[5].length == 3);

    eassert(!memcmp(lexemes.strs[6].value, "|", 2));
    eassert(lexemes.ops[6] == T_PIPE);
    eassert(lexemes.strs[6].length == 2);

    eassert(!memcmp(lexemes.strs[7].value, "wc", 2));
    eassert(lexemes.ops[7] == T_CONST);
    eassert(lexemes.strs[7].length == 3);

    eassert(!memcmp(lexemes.strs[8].value, "-c", 2));
    eassert(lexemes.ops[8] == T_CONST);
    eassert(lexemes.strs[8].length == 3);

    eassert(!lexemes.strs[9].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("longrunningprogram &");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "longrunningprogram", 19));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 19);

    eassert(!memcmp(lexemes.strs[1].value, "&", 2));
    eassert(lexemes.ops[1] == T_AMP);
    eassert(lexemes.strs[1].length == 2);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls > text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, ">", 2));
    eassert(lexemes.ops[1] == T_GT);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "text.txt", 9));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 9);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls >> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, ">", 1));
    eassert(lexemes.ops[1] == T_GT);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, ">", 1));
    eassert(lexemes.ops[2] == T_GT);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, "text.txt", 9));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 9);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("t.txt < sort");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "t.txt", 6));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 6);

    eassert(!memcmp(lexemes.strs[1].value, "<", 1));
    eassert(lexemes.ops[1] == T_LT);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 5));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 5);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls 2> t.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 2));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "2", 1));
    eassert(lexemes.ops[1] == T_NUM);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, ">", 1));
    eassert(lexemes.ops[2] == T_GT);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, "t.txt", 6));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 6);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls &> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "&", 1));
    eassert(lexemes.ops[1] == T_AMP);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, ">", 1));
    eassert(lexemes.ops[2] == T_GT);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, "text.txt", 9));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 9);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls &>> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "&", 1));
    eassert(lexemes.ops[1] == T_AMP);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, ">", 1));
    eassert(lexemes.ops[2] == T_GT);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, ">", 1));
    eassert(lexemes.ops[3] == T_GT);
    eassert(lexemes.strs[3].length == 2);

    eassert(!memcmp(lexemes.strs[4].value, "text.txt", 9));
    eassert(lexemes.ops[4] == T_CONST);
    eassert(lexemes.strs[4].length == 9);

    eassert(!lexemes.strs[5].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=\"Hello\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.strs[0].value, "STR", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 4);

    eassert(!memcmp(lexemes.strs[1].value, "=", 1));
    eassert(lexemes.ops[1] == T_EQ);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "\"", 1));
    eassert(lexemes.ops[2] == T_D_QUOTE);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, "Hello", 6));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 6);

    eassert(!memcmp(lexemes.strs[4].value, "\"", 1));
    eassert(lexemes.ops[4] == T_D_QUOTE);
    eassert(lexemes.strs[4].length == 2);

    eassert(!lexemes.strs[5].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=\"ls -a\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 6);

    eassert(!memcmp(lexemes.strs[0].value, "STR", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 4);

    eassert(!memcmp(lexemes.strs[1].value, "=", 1));
    eassert(lexemes.ops[1] == T_EQ);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "\"", 1));
    eassert(lexemes.ops[2] == T_D_QUOTE);
    eassert(lexemes.strs[2].length == 2);

    eassert(!memcmp(lexemes.strs[3].value, "ls", 2));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 3);

    eassert(!memcmp(lexemes.strs[4].value, "-a", 2));
    eassert(lexemes.ops[4] == T_CONST);
    eassert(lexemes.strs[4].length == 3);

    eassert(!memcmp(lexemes.strs[5].value, "\"", 1));
    eassert(lexemes.ops[5] == T_D_QUOTE);
    eassert(lexemes.strs[5].length == 2);

    eassert(!lexemes.strs[6].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo $STR");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length = 5);

    eassert(!memcmp(lexemes.strs[1].value, "$", 1));
    eassert(lexemes.ops[1] == T_DOLLAR);
    eassert(lexemes.strs[1].length = 2);

    eassert(!memcmp(lexemes.strs[2].value, "STR", 3));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length = 4);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=hello && echo $STR");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 8);

    eassert(!memcmp(lexemes.strs[0].value, "STR", sizeof("STR")));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == sizeof("STR"));

    eassert(!memcmp(lexemes.strs[1].value, "=", sizeof("=")));
    eassert(lexemes.ops[1] == T_EQ);
    eassert(lexemes.strs[1].length == sizeof("="));

    eassert(!memcmp(lexemes.strs[2].value, "hello", sizeof("hello")));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == sizeof("hello"));

    eassert(!memcmp(lexemes.strs[3].value, "&", sizeof("&")));
    eassert(lexemes.ops[3] == T_AMP);
    eassert(lexemes.strs[3].length == sizeof("&"));

    eassert(!memcmp(lexemes.strs[4].value, "&", sizeof("&")));
    eassert(lexemes.ops[4] == T_AMP);
    eassert(lexemes.strs[4].length == sizeof("&"));

    eassert(!memcmp(lexemes.strs[5].value, "echo", sizeof("echo")));
    eassert(lexemes.ops[5] == T_CONST);
    eassert(lexemes.strs[5].length = sizeof("echo"));

    eassert(!memcmp(lexemes.strs[6].value, "$", sizeof("$")));
    eassert(lexemes.ops[6] == T_DOLLAR);
    eassert(lexemes.strs[6].length = sizeof("$"));

    eassert(!memcmp(lexemes.strs[7].value, "STR", sizeof("STR")));
    eassert(lexemes.ops[7] == T_CONST);
    eassert(lexemes.strs[7].length = sizeof("STR"));

    eassert(!lexemes.strs[8].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("COMMAND=ls && $COMMAND");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "COMMAND=ls", sizeof("COMMAND=ls")));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == sizeof("COMMAND=ls"));

    eassert(!memcmp(lexemes.strs[1].value, "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.strs[1].length == 3);

    eassert(!memcmp(lexemes.strs[2].value, "$COMMAND", 8));
    eassert(lexemes.ops[2] == OP_VARIABLE);
    eassert(lexemes.strs[2].length = 8);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo \"hello\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "\"", 1));
    eassert(lexemes.ops[1] == T_D_QUOTE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "hello", 6));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 6);

    eassert(!memcmp(lexemes.strs[3].value, "\"", 1));
    eassert(lexemes.ops[3] == T_D_QUOTE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo \'hello\'");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "'", 1));
    eassert(lexemes.ops[1] == T_QUOTE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "hello", 6));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 6);

    eassert(!memcmp(lexemes.strs[3].value, "'", 1));
    eassert(lexemes.ops[3] == T_QUOTE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo `hello`");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "`", 1));
    eassert(lexemes.ops[1] == T_BACKTICK);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "hello", 6));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 6);

    eassert(!memcmp(lexemes.strs[3].value, "`", 1));
    eassert(lexemes.ops[3] == T_BACKTICK);
    eassert(lexemes.strs[3].length == 2);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("git commit -m \"this is a commit message\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "git", 4));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 4);

    eassert(!memcmp(lexemes.strs[1].value, "commit", 7));
    eassert(lexemes.ops[1] == T_CONST);
    eassert(lexemes.strs[1].length == 7);

    eassert(!memcmp(lexemes.strs[2].value, "-m", 3));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 3);

    eassert(!memcmp(lexemes.strs[3].value, "this is a commit message", 25));
    eassert(lexemes.ops[3] == T_CONST);
    eassert(lexemes.strs[3].length == 25);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ~");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "~", 2));
    eassert(lexemes.ops[1] == T_HOME);
    eassert(lexemes.strs[1].length == 2);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ~/tmp");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "~/tmp", sizeof("~/tmp") - 1));
    eassert(lexemes.ops[1] == T_HOME);
    eassert(lexemes.strs[1].length == sizeof("~/tmp"));

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.ops[0] == OP_MATH_EXPRESSION_START);

    eassert(!memcmp(lexemes.strs[1].value, "1", 1));
    eassert(lexemes.ops[1] == T_NUM);

    eassert(lexemes.ops[2] == OP_ADD);

    eassert(!memcmp(lexemes.strs[3].value, "1", 1));
    eassert(lexemes.ops[3] == T_NUM);

    eassert(lexemes.ops[4] == OP_SUBTRACT);

    eassert(!memcmp(lexemes.strs[5].value, "1", 1));
    eassert(lexemes.ops[5] == T_NUM);

    eassert(lexemes.ops[6] == OP_MULTIPLY);

    eassert(!memcmp(lexemes.strs[7].value, "1", 1));
    eassert(lexemes.ops[7] == T_NUM);

    eassert(lexemes.ops[8] == OP_DIVIDE);

    eassert(!memcmp(lexemes.strs[9].value, "1", 1));
    eassert(lexemes.ops[9] == T_NUM);

    eassert(lexemes.ops[10] == OP_MODULO);

    eassert(!memcmp(lexemes.strs[11].value, "1", 1));
    eassert(lexemes.ops[11] == T_NUM);

    eassert(lexemes.ops[12] == OP_EXPONENTIATION);

    eassert(!memcmp(lexemes.strs[13].value, "1", 1));
    eassert(lexemes.ops[13] == T_NUM);

    eassert(lexemes.ops[14] == OP_MATH_EXPRESSION_END);

    eassert(!lexemes.strs[15].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls *.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "*.md", sizeof("*.md") - 1));
    eassert(lexemes.strs[1].length == sizeof("*.md"));
    eassert(lexemes.ops[1] == T_GLOB);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ?.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "?.md", sizeof("?.md") - 1));
    eassert(lexemes.strs[1].length == sizeof("?.md"));
    eassert(lexemes.ops[1] == T_GLOB);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_Lit("* * * * * * * * * * * * * * * * * *");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 18);
    for (size_t i = 0; i < lexemes.count; ++i) {
        eassert(lexemes.strs[i].value[0] == '*');
        eassert(lexemes.ops[0] == OP_GLOB_EXPANSION);
        eassert(lexemes.strs[0].length == 2);
    }

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line =
        Str_Lit("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_Lit("??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("false && true || false");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.strs[0].value, "false", 6));
    eassert(lexemes.ops[0] == OP_FALSE);
    eassert(lexemes.strs[0].length == 6);

    eassert(!memcmp(lexemes.strs[1].value, "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.strs[1].length == 3);

    eassert(!memcmp(lexemes.strs[2].value, "true", 5));
    eassert(lexemes.ops[2] == OP_TRUE);
    eassert(lexemes.strs[2].length == 5);

    eassert(!memcmp(lexemes.strs[3].value, "||", 3));
    eassert(lexemes.ops[3] == OP_OR);
    eassert(lexemes.strs[3].length == 3);

    eassert(!memcmp(lexemes.strs[4].value, "false", 6));
    eassert(lexemes.ops[4] == OP_FALSE);
    eassert(lexemes.strs[4].length == 6);

    eassert(!lexemes.strs[5].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 14);

    eassert(lexemes.ops[0] == T_IF);
    eassert(lexemes.ops[1] == T_O_BRACK);
    eassert(lexemes.ops[2] == T_NUM);
    eassert(lexemes.ops[3] == T_EQ_A);
    eassert(lexemes.ops[4] == T_NUM);
    eassert(lexemes.ops[5] == T_C_BRACK);
    eassert(lexemes.ops[6] == T_SEMIC);
    eassert(lexemes.ops[7] == T_THEN);
    eassert(lexemes.ops[8] == T_CONST);
    eassert(lexemes.ops[9] == T_QUOTE);
    eassert(lexemes.ops[10] == T_CONST);
    eassert(lexemes.ops[11] == T_QUOTE);
    eassert(lexemes.ops[12] == T_SEMIC);
    eassert(lexemes.ops[13] == T_FI);
    eassert(!lexemes.ops[14])

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 18);

    eassert(lexemes.ops[0] == T_IF);
    eassert(lexemes.ops[1] == T_O_BRACK);
    eassert(lexemes.ops[2] == T_NUM);
    eassert(lexemes.ops[3] == T_EQ_A);
    eassert(lexemes.ops[4] == T_NUM);
    eassert(lexemes.ops[5] == T_C_BRACK);
    eassert(lexemes.ops[6] == T_SEMIC);
    eassert(lexemes.ops[7] == T_THEN);
    eassert(lexemes.ops[8] == T_CONST);
    eassert(lexemes.ops[9] == T_QUOTE);
    eassert(lexemes.ops[10] == T_CONST);
    eassert(lexemes.ops[11] == T_QUOTE);
    eassert(lexemes.ops[12] == T_SEMIC);
    eassert(lexemes.ops[13] == T_ELSE);
    eassert(lexemes.ops[14] == T_CONST);
    eassert(lexemes.ops[15] == T_CONST);
    eassert(lexemes.ops[16] == T_SEMIC);
    eassert(lexemes.ops[17] == T_FI);
    eassert(!lexemes.ops[18])

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_if_elif_elif_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 2 ]; then echo hey; elif [ 2 -lt 1 ]; then echo hiya; else echo hello; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 40);
    size_t i = 0;

    eassert(lexemes.ops[i++] == T_IF);
    eassert(lexemes.ops[i++] == T_O_BRACK);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_EQ_A);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_C_BRACK);
    eassert(lexemes.ops[i++] == T_SEMIC);
    eassert(lexemes.ops[i++] == T_THEN);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_QUOTE);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_QUOTE);
    eassert(lexemes.ops[i++] == T_SEMIC);

    eassert(lexemes.ops[i++] == T_ELIF);
    eassert(lexemes.ops[i++] == T_O_BRACK);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_EQ_A);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_C_BRACK);
    eassert(lexemes.ops[i++] == T_SEMIC);
    eassert(lexemes.ops[i++] == T_THEN);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_SEMIC);

    eassert(lexemes.ops[i++] == T_ELIF);
    eassert(lexemes.ops[i++] == T_O_BRACK);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_LT_A);
    eassert(lexemes.ops[i++] == T_NUM);
    eassert(lexemes.ops[i++] == T_C_BRACK);
    eassert(lexemes.ops[i++] == T_SEMIC);
    eassert(lexemes.ops[i++] == T_THEN);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_SEMIC);

    eassert(lexemes.ops[i++] == T_ELSE);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_CONST);
    eassert(lexemes.ops[i++] == T_SEMIC);
    eassert(lexemes.ops[i++] == T_FI);

    eassert(!lexemes.ops[i])

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo \"hello\" # this is a comment");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "echo", sizeof("echo")));
    eassert(lexemes.ops[0] == T_CONST);
    eassert(lexemes.strs[0].length == sizeof("echo"));

    eassert(!memcmp(lexemes.strs[1].value, "\"", 1));
    eassert(lexemes.ops[1] == T_D_QUOTE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "hello", sizeof("hello")));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == sizeof("hello"));

    eassert(!memcmp(lexemes.strs[3].value, "\"", 1));
    eassert(lexemes.ops[3] == T_D_QUOTE);
    eassert(lexemes.strs[3].length == 2);

    // comment is stripped by the lexer
    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_var_increment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("count=$((count + 1))");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "count=$((count", 14));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT_EXPR);
    eassert(lexemes.strs[0].length == 15);

    eassert(!memcmp(lexemes.strs[1].value, "+", 1));
    eassert(lexemes.ops[1] == OP_ADD);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "1))", 3));
    eassert(lexemes.ops[2] == T_CONST);
    eassert(lexemes.strs[2].length == 4);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lex_while_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("count=1\nwhile [ $count -lt 3 ]; do echo $count\n count=$((count + 1)) done");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 15);

    eassert(!memcmp(lexemes.strs[0].value, "count=1", 7));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == 8);

    eassert(!memcmp(lexemes.strs[1].value, "while", 5));
    eassert(lexemes.ops[1] == OP_WHILE);
    eassert(lexemes.strs[1].length == 6);

    eassert(lexemes.ops[2] == OP_CONDITION_START);

    eassert(!memcmp(lexemes.strs[3].value, "$count", 6));
    eassert(lexemes.ops[3] == OP_VARIABLE);
    eassert(lexemes.strs[3].length == 7);

    eassert(lexemes.ops[4] == OP_LESS_THAN);

    eassert(!memcmp(lexemes.strs[5].value, "3", 1));
    eassert(lexemes.ops[5] == T_CONST);
    eassert(lexemes.strs[5].length == 2);

    eassert(lexemes.ops[6] == OP_CONDITION_END);
    eassert(lexemes.ops[7] == OP_DO);

    eassert(!memcmp(lexemes.strs[8].value, "echo", 4));
    eassert(lexemes.ops[8] == T_CONST);
    eassert(lexemes.strs[8].length == 5);

    eassert(!memcmp(lexemes.strs[9].value, "$count", 6));
    eassert(lexemes.ops[9] == OP_VARIABLE);
    eassert(lexemes.strs[9].length == 7);

    eassert(!memcmp(lexemes.strs[10].value, "count=$((count", 14));
    eassert(lexemes.ops[10] == OP_ASSIGNMENT_EXPR);
    eassert(lexemes.strs[10].length == 15);

    eassert(!memcmp(lexemes.strs[11].value, "+", 1));
    eassert(lexemes.ops[11] == OP_ADD);
    eassert(lexemes.strs[11].length == 2);

    eassert(!memcmp(lexemes.strs[12].value, "1))", 3));
    eassert(lexemes.ops[12] == T_CONST);
    eassert(lexemes.strs[12].length == 4);

    eassert(lexemes.ops[13] == OP_DONE);

    eassert(!lexemes.strs[14].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void lex_bad_input_shouldnt_crash();

void lexer_tests()
{
    // etest_init(true);
    etest_start();

    etest_run(lex_ls_test);
    etest_run(lex_ls_dash_l_test);
    etest_run(lex_pipe_test);
    etest_run(lex_multiple_pipes_test);
    etest_run(lex_multiple_pipes_multiple_args_test);
    etest_run(lex_background_job_test);
    etest_run(lex_output_redirection_test);
    etest_run(lex_double_quotes_test);
    etest_run(lex_single_quotes_test);
    etest_run(lex_backtick_quotes_test);

    etest_run(lex_output_redirection_append_test);
    etest_run(lex_input_redirection_test);
    etest_run(lex_stderr_redirection_test);
    etest_run(lex_stdout_and_stderr_redirection_test);
    etest_run(lex_stdout_and_stderr_redirection_append_test);

    etest_run(lex_assignment_test);
    etest_run(lex_assignment_spaces_test);
    etest_run(lex_variable_test);
    etest_run(lex_variable_and_test);
    /*etest_run(lex_variable_command_test);
    etest_run(lex_git_commit_test);*/
    etest_run(lex_home_test);
    etest_run(lex_home_at_start_test);
    // etest_run(lex_math_operators_test);
    etest_run(lex_glob_star_test);
    etest_run(lex_glob_question_test);
    /*etest_run(lex_glob_star_shouldnt_crash);
    etest_run(lex_tilde_home_shouldnt_crash);
    etest_run(lex_glob_question_and_tilde_home_shouldnt_crash);
    etest_run(lex_bad_input_shouldnt_crash);
    etest_run(lex_bool_test);*/
    etest_run(lex_if_test);
    etest_run(lex_if_else_test);
    etest_run(lex_if_elif_elif_else_test);
    etest_run(lex_comment_test);

    /*etest_run(lex_var_increment_test);
    etest_run(lex_while_test);*/

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
void lex_bad_input_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);

    // not crashing is a test pass
    (void)lexemes;

    SCRATCH_ARENA_TEST_TEARDOWN;
}
