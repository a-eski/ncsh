#include <assert.h>
#include <unistd.h>

#include "../../src/env.h"
#include "../etest.h"
#include "../../src/interpreter/lexer.h"
#include "../../src/interpreter/parser.h"
#include "../lib/arena_test_helper.h"
#include "../lib/shell_test_helper.h"

#define LS Str_New_Literal("ls")
#define SORT Str_New_Literal("sort")
#define DASH_L Str_New_Literal("-l")
#define JOB Str_New_Literal("longrunningprogram")
#define FILE Str_New_Literal("text.txt")
#define ECHO Str_New_Literal("echo")
#define SLEEP Str_New_Literal("sleep")

static char** envp_ptr;

void parser_parse_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = LS;

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, line.value, line.length));
    eassert(cmds->strs[0].length == line.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);

    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls -l");
    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 2);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, DASH_L.value, DASH_L.length - 1));
    eassert(cmds->strs[1].length == DASH_L.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 2);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;

    eassert(cmds->count == 1);
    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);

    cmds = cmds->next;
    eassert(cmds->count == 1);
    eassert(!memcmp(cmds->strs[0].value, SORT.value, SORT.length - 1));
    eassert(cmds->strs[0].length == SORT.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort | table");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 3);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;

    eassert(cmds->count == 1);
    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);

    cmds = cmds->next;
    eassert(cmds->count == 1);
    eassert(!memcmp(cmds->strs[0].value, SORT.value, SORT.length - 1));
    eassert(cmds->strs[0].length == SORT.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[1].value);

    cmds = cmds->next;
    eassert(cmds->count == 1);
    eassert(!memcmp(cmds->strs[0].value, "table", 5));
    eassert(cmds->strs[0].length == 6);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[1].value);

    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_multiple_pipes_multiple_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls | sort | head -1 | wc -c");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 4);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;

    // First command
    eassert(cmds->count == 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);

    // Second command
    cmds = cmds->next;
    eassert(cmds->count == 1);

    eassert(!memcmp(cmds->strs[0].value, SORT.value, SORT.length - 1));
    eassert(cmds->strs[0].length == SORT.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[1].value);

    // Third command
    cmds = cmds->next;
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[0].value, "head", 4));
    eassert(cmds->strs[0].length == 5);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "-1", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[2].value);

    // Fourth command
    cmds = cmds->next;
    eassert(cmds->count == 2);
    eassert(!memcmp(cmds->strs[0].value, "wc", 2));
    eassert(cmds->strs[0].length == 3);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "-c", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(cmds->prev_op == OP_PIPE);

    eassert(!cmds->strs[2].value);

    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("longrunningprogram &");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, JOB.value, JOB.length - 1));
    eassert(cmds->strs[0].length == JOB.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_background_job_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("sleep 5 &");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, SLEEP.value, SLEEP.length - 1));
    eassert(cmds->strs[0].length == SLEEP.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "5", 1));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls > text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_OUT);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls >> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_OUT_APPEND);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("sort < text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_IN);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, SORT.value, SORT.length - 1));
    eassert(cmds->strs[0].length == SORT.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_input_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("sort << text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_IN_APPEND);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, SORT.value, SORT.length - 1));
    eassert(cmds->strs[0].length == SORT.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls 2> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_ERR);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls 2>> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_ERR_APPEND);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls &> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_OUT_ERR);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls &>> text.txt");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);

    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_OUT_ERR_APPEND);
    eassert(stmts->redirect_filename);
    eassert(!memcmp(stmts->redirect_filename, FILE.value, FILE.length - 1));
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_git_commit_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("git commit -m \"this is a commit message\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count = 4);

    eassert(!memcmp(cmds->strs[0].value, "git", 3));
    eassert(cmds->strs[0].length == 4);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "commit", 6));
    eassert(cmds->strs[1].length == 7);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!memcmp(cmds->strs[2].value, "-m", 2));
    eassert(cmds->strs[2].length == 3);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!memcmp(cmds->strs[3].value, "this is a commit message", 24));
    eassert(cmds->strs[3].length == 25);
    eassert(cmds->ops[3] == OP_CONST);

    eassert(!cmds->strs[4].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"Hello\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, &shell, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"ls -a\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, &shell, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("STR=\"ls | sort\"");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, &shell, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 0);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell.arena = a;
    shell.scratch = s;
    env_new(&shell, envp_ptr, &shell.arena);
    *env_add_or_get(shell.env, Str_New_Literal("STR")) = Str_New_Literal("hello");

    auto line = Str_New_Literal("echo $STR");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &s);
    auto res = parser_parse(&lexemes, &shell, &s);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_and_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell.arena = a;
    shell.scratch = s;
    env_new(&shell, envp_ptr, &shell.arena);
    *env_add_or_get(shell.env, Str_New_Literal("STR")) = Str_New_Literal("hello");

    auto line = Str_New_Literal("STR=hello && echo $STR");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &s);
    auto res = parser_parse(&lexemes, &shell, &s);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_variable_command_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell.arena = a;
    shell.scratch = s;
    env_new(&shell, envp_ptr, &shell.arena);
    *env_add_or_get(shell.env, Str_New_Literal("STR")) = Str_New_Literal("hello");

    auto line = Str_New_Literal("COMMAND=ls && $COMMAND");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &s);
    auto res = parser_parse(&lexemes, &shell, &s);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 1);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!cmds->strs[1].value);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ~");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, &shell, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    // first command
    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    // second command
    Str home = Str_Get(getenv(NCSH_HOME_VAL));
    eassert(!memcmp(cmds->strs[1].value, home.value, home.length - 1));
    eassert(cmds->strs[1].length == home.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ~/snap");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, &shell, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    // first command
    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    // construct home expanded value from '~/snap'

    Str home = Str_Get(getenv(NCSH_HOME_VAL));
    char* val = arena_malloc(&scratch_arena, cmds->strs[1].length + home.length - 1, char);
    memcpy(val, home.value, home.length - 1);
    memcpy(val + home.length - 1, "/snap", 5);
    // eassert(!memcmp(val, "/home/alex/snap", 15)); // just make sure constructing home expanded value correctly
    size_t val_len = strlen(val) + 1;

    // second command
    eassert(!memcmp(cmds->strs[1].value, val, val_len - 1));
    eassert(cmds->strs[1].length == val_len);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count > 10);
    // TODO: implement math operations in parser

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls *.md");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 4);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "COMPILE.md", 10));
    eassert(cmds->strs[1].length == 11);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!memcmp(cmds->strs[2].value, "NOTES.md", 8));
    eassert(cmds->strs[2].length == 9);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!memcmp(cmds->strs[3].value, "README.md", 9));
    eassert(cmds->strs[3].length == 10);
    eassert(cmds->ops[3] == OP_CONST);

    eassert(!cmds->strs[4].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls ?OTES.md");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "NOTES.md", 8));
    eassert(cmds->strs[1].length == 9);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_midcommand_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("ls N?TES.md");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[0].value, LS.value, LS.length - 1));
    eassert(cmds->strs[0].length == LS.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "NOTES.md", 8));
    eassert(cmds->strs[1].length == 9);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

/*
// Since some tests can call arena_realloc, we need to take care
// to ensure the scratch arena lifetime is properly managed and not reset
int parser_arena_ctx_wrapper(Str line, stmts-> stmts, Arena* restrict sa)
{
    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, sa);
    int res = parser_parse(&lexemes, stmts, NULL, sa);
    return res;
}

void parser_parse_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    Str line = Str_New_Literal("* * * * * * * * * * * * * * * * * *");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &s);
    stmts->stmts = {0};
    int res = parser_parse(&lexemes, &stmts, NULL, &s);

    eassert(!res);
    eassert(stmts.count == 1);
    eassert(stmts.stmts->>count == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_New_Literal("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    stmts->statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    int res = parser_parse(&lexemes, &stmts-> &shell, &scratch_arena);
    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_New_Literal("??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                                "~~~~?~>w?");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    stmts->statements = {0};
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    int res = parser_parse(&lexemes, &stmts-> &shell, &scratch_arena);

    eassert(!res);

    SCRATCH_ARENA_TEST_TEARDOWN;
}*/

void parser_parse_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("echo \"hello\" # this is a comment");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    Shell shell = {0};
    shell_init(&shell, &scratch_arena, envp_ptr);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("false && true || false");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;

    // first command
    eassert(cmds->count == 1);

    eassert(!memcmp(cmds->strs[0].value, "false", 5));
    eassert(cmds->strs[0].length == 6);
    eassert(cmds->ops[0] == OP_FALSE);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[1].value);

    // second command
    cmds = cmds->next;
    eassert(cmds->count == 1);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, "true", 4));
    eassert(cmds->strs[0].length == 5);
    eassert(cmds->ops[0] == OP_TRUE);
    eassert(cmds->prev_op == OP_AND);

    eassert(!cmds->strs[1].value);

    // third command
    cmds = cmds->next;
    eassert(cmds->count == 1);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, "false", 5));
    eassert(cmds->strs[0].length == 6);
    eassert(cmds->ops[0] == OP_FALSE);
    eassert(cmds->prev_op == OP_OR);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement, conditions
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement, if statements
    stmt = stmt->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);
    // eassert(!commands->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_variable_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell.arena = a;
    shell.scratch = s;
    env_new(&shell, envp_ptr, &shell.arena);
    *env_add_or_get(shell.env, Str_New_Literal("VAL")) = Str_New_Literal("1");

    auto line = Str_New_Literal("if [ $VAL -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &s);
    auto res = parser_parse(&lexemes, &shell, &s);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement, conditions
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement, if statements
    stmt = stmts->head->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);
    // eassert(!commands->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_multiple_conditions_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ true && true ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement, conditions
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 1);

    eassert(!memcmp(cmds->strs[0].value, "true", 4));
    eassert(cmds->strs[0].length == 5);
    eassert(cmds->ops[0] == OP_TRUE);

    eassert(!cmds->strs[1].value);

    cmds = cmds->next;
    eassert(!memcmp(cmds->strs[0].value, "true", 4));
    eassert(cmds->strs[0].length == 5);
    eassert(cmds->ops[0] == OP_TRUE);
    eassert(cmds->prev_op == OP_AND);

    eassert(!cmds->strs[1].value);
    eassert(!cmds->next);

    // second statement, if statements
    stmt = stmt->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);
    // eassert(!commands->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

[[maybe_unused]]
void print_stmt(Statements* stmts)
{
    printf("stmts->type %d\n", stmts->type);

    printf("stmts->head->type %d\n", stmts->head->type);
    printf("stmts->head->right->type %d\n", stmts->head->right->type);
    printf("stmts->head->right->prev->type %d\n", stmts->head->right->prev->type);
    printf("stmts->head->left->type %d\n", stmts->head->left->type);
}

void parser_parse_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF_ELSE);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement, conditions
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement, if statements
    stmt = stmts->head->right;
    eassert(stmt->prev);
    eassert(stmt->prev->type == LT_IF_CONDITIONS);
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    // eassert(!cmds->next); // TODO: fix this.

    // third statement, else statements
    stmt = stmts->head->left;
    eassert(stmt);
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELSE);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);
    // TODO: fix this so commands->next is null and don't need to check strs[0].value
    // eassert(!commands->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_elif_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 2 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF_ELIF);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "2", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement
    stmt = stmts->head->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // fourth statement
    stmt = stmts->head->left->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hey", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_elif_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 2 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; else echo hello; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF_ELIF_ELSE);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "2", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement
    stmt = stmts->head->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // fourth statement
    stmt = stmts->head->left->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hey", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // fifth statement
    stmt = stmts->head->left->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELSE);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_parse_if_elif_multiple_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_New_Literal("if [ 1 -eq 2 ]; then echo 'hi'; elif [ 1 -eq 3 ]; then echo hey; elif [ 1 -eq 1 ]; then echo hallo; else echo hello; fi");

    Lexemes lexemes = {0};
    lexer_lex(line, &lexemes, &scratch_arena);
    auto res = parser_parse(&lexemes, NULL, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(!res.msg);
    eassert(res.stmts);
    auto stmts = res.stmts;
    eassert(stmts->type == ST_IF_ELIF_ELSE);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;

    // first statement
    eassert(stmt->type == LT_IF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "2", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // second statement
    stmt = stmts->head->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_IF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hi", 2));
    eassert(cmds->strs[1].length == 3);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "3", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // fourth statement
    stmt = stmts->head->left->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hey", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // fifth statement
    stmt = stmts->head->left->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQUALS);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_CONST);
    eassert(cmds->prev_op == OP_EQUALS);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    // sixth statement
    stmt = stmts->head->left->left->right;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hallo", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    // seventh statement
    stmt = stmts->head->left->left->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELSE);
    eassert(cmds->count == 2);

    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!memcmp(cmds->strs[1].value, "hello", 5));
    eassert(cmds->strs[1].length == 6);
    eassert(cmds->ops[1] == OP_CONST);
    eassert(cmds->prev_op == OP_NONE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next->strs[0].value);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_tests()
{
    // etest_init(true);
    etest_start();

    etest_run(parser_parse_ls_test);
    etest_run(parser_parse_ls_dash_l_test);

    etest_run(parser_parse_pipe_test);
    etest_run(parser_parse_multiple_pipes_test);
    etest_run(parser_parse_multiple_pipes_multiple_args_test);

    etest_run(parser_parse_background_job_test);
    etest_run(parser_parse_background_job_args_test);

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

    // etest_run(parser_parse_math_operators_test);

    etest_run(parser_parse_glob_star_test);
    etest_run(parser_parse_glob_question_test);
    etest_run(parser_parse_glob_question_midcommand_test);

    // etest_run(parser_parse_glob_star_shouldnt_crash);
    // etest_run(parser_parse_tilde_home_shouldnt_crash);
    // etest_run(parser_parse_glob_question_and_tilde_home_shouldnt_crash);

    etest_run(parser_parse_comment_test);

    etest_run(parser_parse_bool_test);

    etest_run(parser_parse_if_test);
    etest_run(parser_parse_if_variable_test);
    etest_run(parser_parse_if_multiple_conditions_test);
    etest_run(parser_parse_if_else_test);
    etest_run(parser_parse_if_elif_test);
    etest_run(parser_parse_if_elif_else_test);
    etest_run(parser_parse_if_elif_multiple_else_test);

    etest_finish();
}

#ifndef TEST_ALL
int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv,
         char** envp)
{
    envp_ptr = envp;

    parser_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
