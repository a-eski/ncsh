#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/compiler/lexer.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

void lexer_lex_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls\0";
    size_t len = 3;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks);
    eassert(toks->head);
    eassert(toks->count == 1);

    Token* tok = toks->head->next;
    eassert(tok);
    eassert(!memcmp(tok->val, line, len));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == len);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l\0";
    size_t len = 6;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks);
    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "-l", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort\0";
    size_t len = 10;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks);
    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(memcmp(tok->val, "ls", 3) == 0);
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(memcmp(tok->val, "|", 2) == 0);
    eassert(tok->op == OP_PIPE);
    eassert(tok->len == 2);
    tok = tok->next;

    eassert(memcmp(tok->val, "sort", 5) == 0);
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_multiple_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t len = 18;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks);
    eassert(toks->head);
    eassert(toks->count == 5);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "|", 2));
    eassert(tok->op == OP_PIPE);
    eassert(tok->len == 2);
    tok = tok->next;

    eassert(!memcmp(tok->val, "sort", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "|", 2));
    eassert(tok->op == OP_PIPE);
    eassert(tok->len == 2);
    tok = tok->next;

    eassert(!memcmp(tok->val, "table", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &\0";
    size_t len = 21;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks);
    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "longrunningprogram", 19));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 19);
    tok = tok->next;

    eassert(!memcmp(tok->val, "&", 2));
    eassert(tok->op == OP_BACKGROUND_JOB);
    eassert(tok->len == 2);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt\0";
    size_t len = 14;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, ">", 2));
    eassert(tok->op == OP_STDOUT_REDIRECTION);
    eassert(tok->len == 2);
    tok = tok->next;

    eassert(!memcmp(tok->val, "text.txt", 9));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 9);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt\0";
    size_t len = 15;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, ">>", 3));
    eassert(tok->op == OP_STDOUT_REDIRECTION_APPEND);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "text.txt", 9));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 9);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "t.txt < sort";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "t.txt", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);
    tok = tok->next;

    eassert(!memcmp(tok->val, "<", 2));
    eassert(tok->op == OP_STDIN_REDIRECTION);
    eassert(tok->len == 2);
    tok = tok->next;

    eassert(!memcmp(tok->val, "sort", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt\0";
    size_t len = 15;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "&>", 3));
    eassert(tok->op == OP_STDOUT_AND_STDERR_REDIRECTION);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "text.txt", 9));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 9);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "&>>", 4));
    eassert(tok->op == OP_STDOUT_AND_STDERR_REDIRECTION_APPEND);
    eassert(tok->len == 4);
    tok = tok->next;

    eassert(!memcmp(tok->val, "text.txt", 9));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 9);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 1);

    Token* tok = toks->head->next;
    // quotes stripped by the parser
    size_t stripped_len = sizeof("STR=Hello");
    eassert(!memcmp(tok->val, "STR=Hello", stripped_len));
    eassert(tok->op == OP_ASSIGNMENT);
    eassert(tok->len == stripped_len);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls -a\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 1);

    Token* tok = toks->head->next;
    // quotes stripped by the parser
    eassert(!memcmp(tok->val, "STR=ls -a", len - 2));
    eassert(tok->op == OP_ASSIGNMENT);
    eassert(tok->len == len - 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls | sort\"";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 1);

    Token* tok = toks->head->next;
    // quotes stripped by the parser
    eassert(!memcmp(tok->val, "STR=ls | sort", len - 2));
    eassert(tok->op == OP_ASSIGNMENT);
    eassert(tok->len == len - 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo $STR";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len = 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "$STR", 5));
    eassert(tok->op == OP_VARIABLE);
    eassert(tok->len = 5);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && echo $STR";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 4);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "STR=hello", sizeof("STR=hello")));
    eassert(tok->op == OP_ASSIGNMENT);
    eassert(tok->len == sizeof("STR=hello"));
    tok = tok->next;

    eassert(!memcmp(tok->val, "&&", 3));
    eassert(tok->op == OP_AND);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len = 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "$STR", 5));
    eassert(tok->op == OP_VARIABLE);
    eassert(tok->len = 5);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls && $COMMAND";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 3);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "COMMAND=ls", sizeof("COMMAND=ls")));
    eassert(tok->op == OP_ASSIGNMENT);
    eassert(tok->len == sizeof("COMMAND=ls"));
    tok = tok->next;

    eassert(!memcmp(tok->val, "&&", 3));
    eassert(tok->op == OP_AND);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "$COMMAND", 8));
    eassert(tok->op == OP_VARIABLE);
    eassert(tok->len = 8);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_double_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\"\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "hello", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_single_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \'hello\'\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "hello", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_backtick_quotes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo `hello`\0";
    size_t len = 13;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "hello", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 4);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "git", 4));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 4);
    tok = tok->next;

    eassert(!memcmp(tok->val, "commit", 7));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 7);
    tok = tok->next;

    eassert(!memcmp(tok->val, "-m", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "this is a commit message", 25));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 25);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~\0";
    size_t len = 5;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "~", 2));
    eassert(tok->op == OP_HOME_EXPANSION);
    eassert(tok->len == 2);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap";
    size_t len = 10;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "~/snap", sizeof("~/snap") - 1));
    eassert(tok->op == OP_HOME_EXPANSION);
    eassert(tok->len == sizeof("~/snap"));

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t len = strlen(line) + 1;
    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    Token* tok = toks->head->next;
    eassert(tok->op == OP_MATH_EXPRESSION_START);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_ADD);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_SUBTRACT);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_MULTIPLY);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_DIVIDE);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_MODULO);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_EXPONENTIATION);
    tok = tok->next;

    eassert(!memcmp(tok->val, "1", 1));
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;

    eassert(tok->op == OP_MATH_EXPRESSION_END);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls *.md";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "*.md", sizeof("*.md") - 1));
    eassert(tok->op == OP_GLOB_EXPANSION);
    eassert(tok->len == sizeof("*.md"));

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ?.md";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "ls", 3));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "?.md", sizeof("?.md") - 1));
    eassert(tok->op == OP_GLOB_EXPANSION);
    eassert(tok->len == sizeof("?.md"));

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->count == 18);
    Token* tok = toks->head->next;
    for (size_t i = 0; i < toks->count; ++i) {
        eassert(tok->val[0] == '*');
        eassert(tok->op == OP_GLOB_EXPANSION);
        eassert(tok->len == 2);
        tok = tok->next;
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

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                 "~~~~?~>w?";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "false && true || false";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 5);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "false", 6));
    eassert(tok->op == OP_FALSE);
    eassert(tok->len == 6);
    tok = tok->next;

    eassert(!memcmp(tok->val, "&&", 3));
    eassert(tok->op == OP_AND);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "true", 5));
    eassert(tok->op == OP_TRUE);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "||", 3));
    eassert(tok->op == OP_OR);
    eassert(tok->len == 3);
    tok = tok->next;

    eassert(!memcmp(tok->val, "false", 6));
    eassert(tok->op == OP_FALSE);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 10);

    Token* tok = toks->head->next;
    eassert(tok);
    eassert(tok->op == OP_IF);
    tok = tok->next;
    eassert(tok->op == OP_CONDITION_START);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_EQUALS);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_CONDITION_END);
    tok = tok->next;
    eassert(tok->op == OP_THEN);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_FI);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 13);

    Token* tok = toks->head->next;
    eassert(tok);
    eassert(tok->op == OP_IF);
    tok = tok->next;
    eassert(tok->op == OP_CONDITION_START);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_EQUALS);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_CONDITION_END);
    tok = tok->next;
    eassert(tok->op == OP_THEN);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_ELSE);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_CONSTANT);
    tok = tok->next;
    eassert(tok->op == OP_FI);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void lexer_lex_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\" # this is a comment\0";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    eassert(toks->head);
    eassert(toks->count == 2);

    Token* tok = toks->head->next;
    eassert(!memcmp(tok->val, "echo", 5));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 5);
    tok = tok->next;

    eassert(!memcmp(tok->val, "hello", 6));
    eassert(tok->op == OP_CONSTANT);
    eassert(tok->len == 6);

    eassert(!tok->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

// forward declaration: implementation put at the end because it messes with clangd lsp
void lexer_lex_bad_input_shouldnt_crash();

int main()
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

    return EXIT_SUCCESS;
}

// put at the end because it messes with clangd lsp
void lexer_lex_bad_input_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~C~~~~~~~~~~~~~k~"
                 "~~~~>ÿÿ> >ÿ>\w\>ÿ> >ÿ> \> >";
    size_t len = strlen(line) + 1;

    Tokens* toks = lexer_lex(line, len, &scratch_arena);

    // hits limit so does not process, not crashing is a test pass
    (void)toks;

    SCRATCH_ARENA_TEST_TEARDOWN;
}
