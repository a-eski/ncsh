#include <unistd.h>

#include "../src/env.h"
#include "../src/eskilib/etest.h"
#include "../src/interpreter/lexer.h"
#include "../src/interpreter/parser.h"
#include "lib/arena_test_helper.h"

#define LS "ls"
#define SORT "sort"
#define DASH_L "-l"
#define JOB "longrunningprogram"
#define FILE "text.txt"
#define ECHO "echo"

constexpr size_t ls_len = sizeof(LS);
constexpr size_t sort_len = sizeof(SORT);
constexpr size_t dashl_len = sizeof(DASH_L);
constexpr size_t job_len = sizeof(JOB);
constexpr size_t file_len = sizeof(FILE);
constexpr size_t echo_len = sizeof(ECHO);

void shell_init(Shell* rst shell, Arena* scratch)
{
    shell->arena = *scratch;
    shell->scratch_arena = *scratch;
    vars_malloc(&shell->arena, &shell->vars);
}

void parser_parse_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = LS;
    size_t len = ls_len;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], line, len));
    eassert(statements.statements->commands->lens[0] == len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls -l";
    size_t len = sizeof("ls -l");

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], DASH_L, dashl_len - 1));
    eassert(statements.statements->commands->lens[1] == dashl_len);
    eassert(statements.statements->commands->ops[1] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort";
    size_t len = sizeof("ls | sort");

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 3);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "|", 1));
    eassert(statements.statements->commands->lens[1] == 2);
    eassert(statements.statements->commands->ops[1] == OP_PIPE);

    eassert(!memcmp(statements.statements->commands->vals[2], SORT, sort_len - 1));
    eassert(statements.statements->commands->lens[2] == sort_len);
    eassert(statements.statements->commands->ops[2] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[3]);

    eassert(statements.pipes_count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls | sort | table";
    size_t len = 18;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 5);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "|", 1));
    eassert(statements.statements->commands->lens[1] == 2);
    eassert(statements.statements->commands->ops[1] == OP_PIPE);

    eassert(!memcmp(statements.statements->commands->vals[2], SORT, sort_len - 1));
    eassert(statements.statements->commands->lens[2] == sort_len);
    eassert(statements.statements->commands->ops[2] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[3], "|", 1));
    eassert(statements.statements->commands->lens[3] == 2);
    eassert(statements.statements->commands->ops[3] == OP_PIPE);

    eassert(!memcmp(statements.statements->commands->vals[4], "table", 5));
    eassert(statements.statements->commands->lens[4] == 6);
    eassert(statements.statements->commands->ops[4] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[5]);

    eassert(statements.pipes_count == 2);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "longrunningprogram &";
    size_t len = sizeof("longrunningprogram &");

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], JOB, job_len - 1));
    eassert(statements.statements->commands->lens[0] == job_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.is_bg_job);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls > text.txt";
    size_t len = 14;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_OUT);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls >> text.txt";
    size_t len = 15;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_OUT_APPEND);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "text.txt < sort";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], SORT, sort_len - 1));
    eassert(statements.statements->commands->lens[0] == sort_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_IN);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "text.txt << sort";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], SORT, sort_len - 1));
    eassert(statements.statements->commands->lens[0] == sort_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_IN_APPEND);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls 2> text.txt";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_ERR);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls 2>> text.txt";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_ERR_APPEND);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &> text.txt";
    size_t len = 15;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_OUT_ERR);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls &>> text.txt";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 1);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[1]);

    eassert(statements.redirect_type == RT_OUT_ERR_APPEND);
    eassert(statements.redirect_filename);
    eassert(!memcmp(statements.redirect_filename, FILE, file_len - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "git commit -m \"this is a commit message\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 4);

    eassert(!memcmp(statements.statements->commands->vals[0], "git", 3));
    eassert(statements.statements->commands->lens[0] == 4);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "commit", 6));
    eassert(statements.statements->commands->lens[1] == 7);
    eassert(statements.statements->commands->ops[1] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[2], "-m", 2));
    eassert(statements.statements->commands->lens[2] == 3);
    eassert(statements.statements->commands->ops[2] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[3], "this is a commit message", 24));
    eassert(statements.statements->commands->lens[3] == 25);
    eassert(statements.statements->commands->ops[3] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[4]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}


void parser_parse_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"Hello\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 0);
    eassert(statements.statements->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls -a\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 0);
    eassert(statements.statements->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=\"ls | sort\"";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 0);
    eassert(statements.statements->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo $STR";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], ECHO, echo_len - 1));
    eassert(statements.statements->commands->lens[0] == echo_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    // TODO: variable replacement
    eassert(!memcmp(statements.statements->commands->vals[1], "$STR", 4));
    eassert(statements.statements->commands->lens[1] == 5);
    eassert(statements.statements->commands->ops[1] == OP_VARIABLE);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_and_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "STR=hello && echo $STR";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], ECHO, echo_len - 1));
    eassert(statements.statements->commands->lens[0] == echo_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    // TODO: variable replacement
    eassert(!memcmp(statements.statements->commands->vals[1], "$STR", 4));
    eassert(statements.statements->commands->lens[1] == 5);
    eassert(statements.statements->commands->ops[1] == OP_VARIABLE);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_command_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "COMMAND=ls && $COMMAND";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena);
    int res = parser_parse(&lexemes, &statements, &shell, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    // eassert(statements.statements->count == 1);
    // eassert(statements.statements->commands->count == 1);

    // TODO: variable replacement
    /*eassert(!memcmp(statements.statements[0].commands->vals[0], "$COMMAND", 8));
    eassert(statements.statements[0].commands->lens[0] == 9);
    eassert(statements.statements[0].commands->ops[0] == OP_VARIABLE);

    eassert(!statements.statements->commands->vals[1]);*/

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~";
    size_t len = 5;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    // first command
    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    // second command
    Str home = {0};
    env_home_get(&home, &scratch_arena);
    eassert(!memcmp(statements.statements->commands->vals[1], home.value, home.length - 1));
    eassert(statements.statements->commands->lens[1] == home.length);
    eassert(statements.statements->commands->ops[1] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ~/snap";
    size_t len = 10;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    // first command
    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    // construct home expanded value from '~/snap'
    Str home = {0};
    env_home_get(&home, &scratch_arena);
    char* val = arena_malloc(&scratch_arena, statements.statements->commands->lens[1] + home.length - 1, char);
    memcpy(val, home.value, home.length - 1);
    memcpy(val + home.length - 1, "/snap", 5);
    // eassert(!memcmp(val, "/home/alex/snap", 15)); // just make sure constructing home expanded value correctly
    size_t val_len = strlen(val) + 1;

    // second command
    eassert(!memcmp(statements.statements->commands->vals[1], val, val_len - 1));
    eassert(statements.statements->commands->lens[1] == val_len);
    eassert(statements.statements->commands->ops[1] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls *.md";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "*.md", 4));
    eassert(statements.statements->commands->lens[1] == 5);
    eassert(statements.statements->commands->ops[1] == OP_GLOB_EXPANSION);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "ls ?.md";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], LS, ls_len - 1));
    eassert(statements.statements->commands->lens[0] == ls_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "?.md", 4));
    eassert(statements.statements->commands->lens[1] == 5);
    eassert(statements.statements->commands->ops[1] == OP_GLOB_EXPANSION);

    eassert(!statements.statements->commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line = "* * * * * * * * * * * * * * * * * *";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 18);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    char* line =
        "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);
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

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "false && true || false";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);
    eassert(statements.statements->count == 1);

    // first command
    Commands* commands = statements.statements->commands;
    eassert(commands->count == 1);

    eassert(!memcmp(commands->vals[0], "false", 5));
    eassert(commands->lens[0] == 6);
    eassert(commands->ops[0] == OP_FALSE);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[1]);

    // second command
    commands = commands->next;
    eassert(commands->count == 1);

    eassert(commands->vals[0]);
    eassert(!memcmp(commands->vals[0], "true", 4));
    eassert(commands->lens[0] == 5);
    eassert(commands->ops[0] == OP_TRUE);
    eassert(commands->prev_op == OP_AND);

    eassert(!commands->vals[1]);

    // third command
    commands = commands->next;
    eassert(commands->count == 1);

    eassert(commands->vals[0]);
    eassert(!memcmp(commands->vals[0], "false", 5));
    eassert(commands->lens[0] == 6);
    eassert(commands->ops[0] == OP_FALSE);
    eassert(commands->prev_op == OP_OR);

    eassert(!commands->vals[1]);
    eassert(!commands->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 2); // should this be 2 instead of 1? there are 2 statements each with 1 set of commands

    // first statement
    Commands* commands = statements.statements[0].commands;
    eassert(statements.statements[0].count == 1);
    eassert(statements.statements[0].type == ST_CONDITIONS);
    eassert(commands->count == 3);

    // first statement, first command
    eassert(!memcmp(commands->vals[0], "1", 1));
    eassert(commands->lens[0] == 2);
    eassert(commands->ops[0] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    // first statement, second command
    eassert(!memcmp(commands->vals[1], "-eq", 3));
    eassert(commands->lens[1] == 4);
    eassert(commands->ops[1] == OP_EQUALS);
    eassert(commands->prev_op == OP_NONE);

    // first statement, first command
    eassert(!memcmp(commands->vals[2], "1", 1));
    eassert(commands->lens[2] == 2);
    eassert(commands->ops[2] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[3]);

    // second statement
    commands = statements.statements[1].commands;
    eassert(statements.statements[1].count == 1);
    eassert(statements.statements[1].type == ST_IF);
    eassert(commands->count == 2);

    // second statement, first command
    eassert(commands->vals[0]);
    eassert(!memcmp(commands->vals[0], ECHO, echo_len - 1));
    eassert(commands->lens[0] == echo_len);
    eassert(commands->ops[0] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    // second statement, second command
    eassert(!memcmp(commands->vals[1], "hi", 2));
    eassert(commands->lens[1] == 3);
    eassert(commands->ops[1] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    eassert(statements.count == 1);

    // first statement
    Commands* commands = statements.statements[0].commands;
    eassert(statements.statements[0].count == 1);
    eassert(statements.statements[0].type == ST_CONDITIONS);
    eassert(commands->count == 3);

    // first statement, first command
    eassert(!memcmp(commands->vals[0], "1", 1));
    eassert(commands->lens[0] == 2);
    eassert(commands->ops[0] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    // first statement, second command
    eassert(!memcmp(commands->vals[1], "-eq", 3));
    eassert(commands->lens[1] == 4);
    eassert(commands->ops[1] == OP_EQUALS);
    eassert(commands->prev_op == OP_NONE);

    // first statement, first command
    eassert(!memcmp(commands->vals[2], "1", 1));
    eassert(commands->lens[2] == 2);
    eassert(commands->ops[2] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[3]);

    // second statement
    commands = statements.statements[1].commands;
    // eassert(statements.statements[1].count == 1);
    eassert(statements.statements[1].type == ST_IF);
    eassert(commands->count == 2);

    // second statement, first command
    eassert(commands->vals[0]);
    eassert(!memcmp(commands->vals[0], ECHO, echo_len - 1));
    eassert(commands->lens[0] == echo_len);
    eassert(commands->ops[0] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    // second statement, second command
    eassert(!memcmp(commands->vals[1], "hi", 2));
    eassert(commands->lens[1] == 3);
    eassert(commands->ops[1] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[2]);

    // third statement
    commands = statements.statements[2].commands;
    eassert(statements.statements[2].count == 1);
    eassert(statements.statements[2].type == ST_ELSE);
    eassert(commands->count == 2);

    // third statement, first command
    eassert(commands->vals[0]);
    eassert(!memcmp(commands->vals[0], ECHO, echo_len - 1));
    eassert(commands->lens[0] == echo_len);
    eassert(commands->ops[0] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    // third statement, second command
    eassert(!memcmp(commands->vals[1], "hello", 5));
    eassert(commands->lens[1] == 6);
    eassert(commands->ops[1] == OP_CONSTANT);
    eassert(commands->prev_op == OP_NONE);

    eassert(!commands->vals[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    char* line = "echo \"hello\" # this is a comment";
    size_t len = strlen(line) + 1;

    Lexemes lexemes = {0};
    lexer_lex(line, len, &lexemes, &scratch_arena);
    Statements statements = {0};
    int res = parser_parse(&lexemes, &statements, NULL, &scratch_arena);

    eassert(!res);
    // eassert(statements.count == 1);
    eassert(statements.statements->count == 1);
    eassert(statements.statements->commands->count == 2);

    eassert(!memcmp(statements.statements->commands->vals[0], ECHO, echo_len - 1));
    eassert(statements.statements->commands->lens[0] == echo_len);
    eassert(statements.statements->commands->ops[0] == OP_CONSTANT);

    eassert(!memcmp(statements.statements->commands->vals[1], "hello", 5));
    eassert(statements.statements->commands->lens[1] == 6);
    eassert(statements.statements->commands->ops[1] == OP_CONSTANT);

    eassert(!statements.statements->commands->vals[2]);

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
    etest_run(parser_parse_output_redirection_append_test);
    etest_run(parser_parse_input_redirection_test);
    etest_run(parser_parse_input_redirection_append_test);
    etest_run(parser_parse_stderr_redirection_test);
    etest_run(parser_parse_stderr_redirection_append_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_test);
    etest_run(parser_parse_stdout_and_stderr_redirection_append_test);

    etest_run(parser_parse_git_commit_test);

    etest_run(parser_parse_assignment_test);
    etest_run(parser_parse_assignment_spaces_test);
    etest_run(parser_parse_assignment_spaces_multiple_test);

    etest_run(parser_parse_variable_test);
    etest_run(parser_parse_variable_and_test);
    etest_run(parser_parse_variable_command_test);

    etest_run(parser_parse_home_test);
    etest_run(parser_parse_home_at_start_test);

    etest_run(parser_parse_math_operators_test);

    etest_run(parser_parse_glob_star_test);
    etest_run(parser_parse_glob_question_test);
    etest_run(parser_parse_glob_star_shouldnt_crash);

    etest_run(parser_parse_tilde_home_shouldnt_crash);
    etest_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);

    etest_run(parser_parse_bool_test);
    etest_run(parser_parse_if_test);
    etest_run(parser_parse_if_else_test);
    etest_run(parser_parse_comment_test);

    etest_finish();

    return EXIT_SUCCESS;
}
