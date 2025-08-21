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

    auto line = Str_New_Literal("ls");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    eassert(lexemes.strs[0].value);
    eassert(!memcmp(lexemes.strs[0].value, line.value, line.length));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == line.length);

    eassert(!lexemes.strs[1].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls -l");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "-l", 3));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 3);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(memcmp(lexemes.strs[0].value, "ls", 3) == 0);
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(memcmp(lexemes.strs[1].value, "|", 2) == 0);
    eassert(lexemes.ops[1] == OP_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(memcmp(lexemes.strs[2].value, "sort", 5) == 0);
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 5);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_multiple_pipes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort | table");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 5);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "|", 2));
    eassert(lexemes.ops[1] == OP_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 5);

    eassert(!memcmp(lexemes.strs[3].value, "|", 2));
    eassert(lexemes.ops[3] == OP_PIPE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!memcmp(lexemes.strs[4].value, "table", 6));
    eassert(lexemes.ops[4] == OP_CONSTANT);
    eassert(lexemes.strs[4].length == 6);

    eassert(!lexemes.strs[5].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_multiple_pipes_multiple_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort | head -1 | wc -c");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 9);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 2));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(memcmp(lexemes.strs[1].value, "|", 2) == 0);
    eassert(lexemes.ops[1] == OP_PIPE);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 4));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 5);

    eassert(!memcmp(lexemes.strs[3].value, "|", 2));
    eassert(lexemes.ops[3] == OP_PIPE);
    eassert(lexemes.strs[3].length == 2);

    eassert(!memcmp(lexemes.strs[4].value, "head", 4));
    eassert(lexemes.ops[4] == OP_CONSTANT);
    eassert(lexemes.strs[4].length == 5);

    eassert(!memcmp(lexemes.strs[5].value, "-1", 2));
    eassert(lexemes.ops[5] == OP_CONSTANT);
    eassert(lexemes.strs[5].length == 3);

    eassert(!memcmp(lexemes.strs[6].value, "|", 2));
    eassert(lexemes.ops[6] == OP_PIPE);
    eassert(lexemes.strs[6].length == 2);

    eassert(!memcmp(lexemes.strs[7].value, "wc", 2));
    eassert(lexemes.ops[7] == OP_CONSTANT);
    eassert(lexemes.strs[7].length == 3);

    eassert(!memcmp(lexemes.strs[8].value, "-c", 2));
    eassert(lexemes.ops[8] == OP_CONSTANT);
    eassert(lexemes.strs[8].length == 3);

    eassert(!lexemes.strs[9].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("longrunningprogram &");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "longrunningprogram", 19));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 19);

    eassert(!memcmp(lexemes.strs[1].value, "&", 2));
    eassert(lexemes.ops[1] == OP_BACKGROUND_JOB);
    eassert(lexemes.strs[1].length == 2);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls > text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, ">", 2));
    eassert(lexemes.ops[1] == OP_STDOUT_REDIRECTION);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 9);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls >> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, ">>", 3));
    eassert(lexemes.ops[1] == OP_STDOUT_REDIRECTION_APPEND);
    eassert(lexemes.strs[1].length == 3);

    eassert(!memcmp(lexemes.strs[2].value, "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 9);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("t.txt < sort");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "t.txt", 6));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 6);

    eassert(!memcmp(lexemes.strs[1].value, "<", 2));
    eassert(lexemes.ops[1] == OP_STDIN_REDIRECTION);
    eassert(lexemes.strs[1].length == 2);

    eassert(!memcmp(lexemes.strs[2].value, "sort", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 5);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls &> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "&>", 3));
    eassert(lexemes.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION);
    eassert(lexemes.strs[1].length == 3);

    eassert(!memcmp(lexemes.strs[2].value, "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 9);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls &>> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 3);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "&>>", 4));
    eassert(lexemes.ops[1] == OP_STDOUT_AND_STDERR_REDIRECTION_APPEND);
    eassert(lexemes.strs[1].length == 4);

    eassert(!memcmp(lexemes.strs[2].value, "text.txt", 9));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 9);

    eassert(!lexemes.strs[3].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"Hello\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    size_t stripped_len = sizeof("STR=Hello");
    eassert(!memcmp(lexemes.strs[0].value, "STR=Hello", stripped_len));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == stripped_len);

    eassert(!lexemes.strs[1].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"ls -a\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    eassert(!memcmp(lexemes.strs[0].value, "STR=ls -a", line.length - 2));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == line.length - 2);

    eassert(!lexemes.strs[1].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"ls | sort\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    // quotes stripped by the lexer
    eassert(!memcmp(lexemes.strs[0].value, "STR=ls | sort", line.length - 2));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == line.length - 2);

    eassert(!lexemes.strs[1].value)

        SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("echo $STR");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length = 5);

    eassert(!memcmp(lexemes.strs[1].value, "$STR", 5));
    eassert(lexemes.ops[1] == OP_VARIABLE);
    eassert(lexemes.strs[1].length = 5);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=hello && echo $STR");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "STR=hello", sizeof("STR=hello")));
    eassert(lexemes.ops[0] == OP_ASSIGNMENT);
    eassert(lexemes.strs[0].length == sizeof("STR=hello"));

    eassert(!memcmp(lexemes.strs[1].value, "&&", 3));
    eassert(lexemes.ops[1] == OP_AND);
    eassert(lexemes.strs[1].length == 3);

    eassert(!memcmp(lexemes.strs[2].value, "echo", 5));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length = 5);

    eassert(!memcmp(lexemes.strs[3].value, "$STR", 5));
    eassert(lexemes.ops[3] == OP_VARIABLE);
    eassert(lexemes.strs[3].length = 5);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("COMMAND=ls && $COMMAND");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

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

void lexer_lex_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("echo \"hello\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 6);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("echo \'hello\'");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 6);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("echo `hello`");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 6);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("git commit -m \"this is a commit message\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 4);

    eassert(!memcmp(lexemes.strs[0].value, "git", 4));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 4);

    eassert(!memcmp(lexemes.strs[1].value, "commit", 7));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 7);

    eassert(!memcmp(lexemes.strs[2].value, "-m", 3));
    eassert(lexemes.ops[2] == OP_CONSTANT);
    eassert(lexemes.strs[2].length == 3);

    eassert(!memcmp(lexemes.strs[3].value, "this is a commit message", 25));
    eassert(lexemes.ops[3] == OP_CONSTANT);
    eassert(lexemes.strs[3].length == 25);

    eassert(!lexemes.strs[4].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ~");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "~", 2));
    eassert(lexemes.ops[1] == OP_HOME_EXPANSION);
    eassert(lexemes.strs[1].length == 2);

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ~/snap");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "~/snap", sizeof("~/snap") - 1));
    eassert(lexemes.ops[1] == OP_HOME_EXPANSION);
    eassert(lexemes.strs[1].length == sizeof("~/snap"));

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.ops[0] == OP_MATH_EXPRESSION_START);

    eassert(!memcmp(lexemes.strs[1].value, "1", 1));
    eassert(lexemes.ops[1] == OP_CONSTANT);

    eassert(lexemes.ops[2] == OP_ADD);

    eassert(!memcmp(lexemes.strs[3].value, "1", 1));
    eassert(lexemes.ops[3] == OP_CONSTANT);

    eassert(lexemes.ops[4] == OP_SUBTRACT);

    eassert(!memcmp(lexemes.strs[5].value, "1", 1));
    eassert(lexemes.ops[5] == OP_CONSTANT);

    eassert(lexemes.ops[6] == OP_MULTIPLY);

    eassert(!memcmp(lexemes.strs[7].value, "1", 1));
    eassert(lexemes.ops[7] == OP_CONSTANT);

    eassert(lexemes.ops[8] == OP_DIVIDE);

    eassert(!memcmp(lexemes.strs[9].value, "1", 1));
    eassert(lexemes.ops[9] == OP_CONSTANT);

    eassert(lexemes.ops[10] == OP_MODULO);

    eassert(!memcmp(lexemes.strs[11].value, "1", 1));
    eassert(lexemes.ops[11] == OP_CONSTANT);

    eassert(lexemes.ops[12] == OP_EXPONENTIATION);

    eassert(!memcmp(lexemes.strs[13].value, "1", 1));
    eassert(lexemes.ops[13] == OP_CONSTANT);

    eassert(lexemes.ops[14] == OP_MATH_EXPRESSION_END);

    eassert(!lexemes.strs[15].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls *.md");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "*.md", sizeof("*.md") - 1));
    eassert(lexemes.ops[1] == OP_GLOB_EXPANSION);
    eassert(lexemes.strs[1].length == sizeof("*.md"));

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ?.md");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "ls", 3));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 3);

    eassert(!memcmp(lexemes.strs[1].value, "?.md", sizeof("?.md") - 1));
    eassert(lexemes.ops[1] == OP_GLOB_EXPANSION);
    eassert(lexemes.strs[1].length == sizeof("?.md"));

    eassert(!lexemes.strs[2].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_New_Literal("* * * * * * * * * * * * * * * * * *");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 18);
    for (size_t i = 0; i < lexemes.count; ++i) {
        eassert(lexemes.strs[i].value[0] == '*');
        eassert(lexemes.ops[0] == OP_GLOB_EXPANSION);
        eassert(lexemes.strs[0].length == 2);
    }

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line =
        Str_New_Literal("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_New_Literal("??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("false && true || false");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

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

void lexer_lex_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

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

    auto line = Str_New_Literal("if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

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

    auto line = Str_New_Literal("echo \"hello\" # this is a comment");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    eassert(lexemes.count == 2);

    eassert(!memcmp(lexemes.strs[0].value, "echo", 5));
    eassert(lexemes.ops[0] == OP_CONSTANT);
    eassert(lexemes.strs[0].length == 5);

    eassert(!memcmp(lexemes.strs[1].value, "hello", 6));
    eassert(lexemes.ops[1] == OP_CONSTANT);
    eassert(lexemes.strs[1].length == 6);

    // comment is stripped by the lexer
    eassert(!lexemes.strs[2].value);

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
    etest_run(lexer_lex_multiple_pipes_test);
    etest_run(lexer_lex_multiple_pipes_multiple_args_test);
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

    auto line = Str_New_Literal("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);

    // not crashing is a test pass
    (void)lexemes;

    SCRATCH_ARENA_TEST_TEARDOWN;
}
