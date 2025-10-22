#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "../etest.h"
#include "../../src/interpreter/lex.h"
#include "../../src/interpreter/parse.h"
#include "../lib/arena_test_helper.h"
#include "../lib/test_defines.h"

static char** envp_ptr;

/* tests start */
void parse_ls_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = LS;

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

void parse_ls_dash_l_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls -l");
    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_pipe_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

void parse_multiple_pipes_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort | table");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

void parse_multiple_pipes_multiple_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls | sort | head -1 | wc -c");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[1] == OP_NUM);

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

void parse_background_job_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("longrunningprogram &");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_background_job_args_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("sleep 5 &");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[1] == OP_NUM);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_output_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls > text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_output_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls >> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_input_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("sort < text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_input_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("sort << text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls 2> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls 2>> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_stdout_and_stderr_redirection_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls &> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_stdout_and_stderr_redirection_append_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls &>> text.txt");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);

    auto stmts = res.output.stmts;
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

void parse_assignment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=\"Hello\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 3);
    eassert(cmds->op == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[0].value, STR.value, STR.length));
    eassert(cmds->strs[0].length == STR.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "=", 2));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[2].value, "Hello", 5));
    eassert(cmds->strs[2].length == 6);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_assignment_spaces_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=\"ls -a\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 3);
    eassert(cmds->op == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[0].value, STR.value, STR.length));
    eassert(cmds->strs[0].length == STR.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "=", 2));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[2].value, "ls -a", sizeof("ls -a")));
    eassert(cmds->strs[2].length == sizeof("ls -a"));
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_assignment_spaces_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=\"ls | sort\"");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 3);
    eassert(cmds->op == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[0].value, STR.value, STR.length));
    eassert(cmds->strs[0].length == STR.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "=", 2));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[2].value, "ls | sort", sizeof("ls | sort")));
    eassert(cmds->strs[2].length == sizeof("ls | sort"));
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_variable_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo $STR");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[1].value, STR.value, STR.length));
    eassert(cmds->strs[1].length == STR.length);
    eassert(cmds->ops[1] == OP_VARIABLE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_variable_and_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("STR=hello && echo $STR");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 3);
    eassert(cmds->op == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[0].value, STR.value, STR.length));
    eassert(cmds->strs[0].length == STR.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, "=", 2));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[2].value, "hello", 5));
    eassert(cmds->strs[2].length == 6);
    eassert(cmds->ops[2] == OP_CONST);

    eassert(!cmds->strs[3].value);

    cmds = cmds->next;
    eassert(cmds);
    eassert(cmds->op == OP_NONE);
    eassert(cmds->prev_op == OP_AND);

    eassert(!memcmp(cmds->strs[0].value, ECHO.value, ECHO.length));
    eassert(cmds->strs[0].length == ECHO.length);
    eassert(cmds->ops[0] == OP_CONST);

    eassert(!memcmp(cmds->strs[1].value, STR.value, STR.length));
    eassert(cmds->strs[1].length == STR.length);
    eassert(cmds->ops[1] == OP_VARIABLE);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_home_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ~");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(!memcmp(cmds->strs[1].value, "~", 1));
    eassert(cmds->strs[1].length == 2);
    eassert(cmds->ops[1] == OP_HOME_EXPANSION);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_home_at_start_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ~/snap");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[1].value, "~/snap", sizeof("~/snap")));
    eassert(cmds->strs[1].length == sizeof("~/snap"));
    eassert(cmds->ops[1] == OP_HOME_EXPANSION);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_glob_star_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls *.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[1].value, "*.md", 4));
    eassert(cmds->strs[1].length == 5);
    eassert(cmds->ops[1] == OP_GLOB_EXPANSION);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_glob_question_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls ?OTES.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[1].value, "?OTES.md", 8));
    eassert(cmds->strs[1].length == 9);
    eassert(cmds->ops[1] == OP_GLOB_EXPANSION);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_glob_question_midcommand_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("ls N?TES.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[1].value, "N?TES.md", 8));
    eassert(cmds->strs[1].length == 9);
    eassert(cmds->ops[1] == OP_GLOB_EXPANSION);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}


// Since some tests can call arena_realloc, we need to take care
// to ensure the scratch arena lifetime is properly managed and not reset
Parser_Output parser_arena_ctx_wrapper(Str line, Arena* restrict sa)
{
    Lexemes lexemes = {0};
    lex(line, &lexemes, sa);
    return parse(&lexemes, sa);
}

void parse_glob_star_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    Str line = Str_Lit("* * * * * * * * * * * * * * * * * *");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    eassert(res.output.stmts->head->commands);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_Lit("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?~");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    eassert(res.output.stmts->head->commands);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_glob_question_and_tilde_home_shouldnt_crash()
{
    SCRATCH_ARENA_TEST_SETUP;

    // found from fuzzer crashing inputs
    auto line = Str_Lit("??~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
                                "~~~~?~>w?");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    eassert(res.output.stmts->head->commands);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_comment_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("echo \"hello\" # this is a comment");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

void parse_bool_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("false && true || false");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

void parse_if_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    if (res.parser_errno) {
        printf("%s\n", res.output.msg);
    }
    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_eq_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 2 -eq 3 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[0].value, "2", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "3", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_variable_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ $VAL -eq 1 ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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

    eassert(!memcmp(cmds->strs[0].value, "VAL", sizeof("VAL")));
    eassert(cmds->strs[0].length == 4);
    eassert(cmds->ops[0] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_multiple_conditions_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ true && true ]; then echo 'hi'; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; else echo hello; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_elif_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    if (res.parser_errno) {
        printf("%s\n", res.output.msg);
    }
    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_elif_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 1 ]; then echo hey; else echo hello; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_if_elif_multiple_else_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("if [ 1 -eq 1 ]; then echo 'hi'; elif [ 1 -eq 3 ]; then echo hey; elif [ 1 -eq 1 ]; then echo hallo; else echo hello; fi");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
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
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    // third statement
    stmt = stmts->head->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "3", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

    // fifth statement
    stmt = stmts->head->left->left;
    cmds = stmt->commands;
    eassert(stmt->type == LT_ELIF_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[0].value, "1", 1));
    eassert(cmds->strs[0].length == 2);
    eassert(cmds->ops[0] == OP_NUM);

    eassert(!memcmp(cmds->strs[1].value, "-eq", 3));
    eassert(cmds->strs[1].length == 4);
    eassert(cmds->ops[1] == OP_EQ_A);

    eassert(!memcmp(cmds->strs[2].value, "1", 1));
    eassert(cmds->strs[2].length == 2);
    eassert(cmds->ops[2] == OP_NUM);

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
    eassert(!cmds->next);

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
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_math_operators_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("$( 1 + 1 - 1 * 1 / 1 % 1 ** 1 )");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_NORMAL);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head->type == LT_NORMAL);
    eassert(stmts->head->commands);
    auto cmds = stmts->head->commands;
    eassert(cmds->count == 15);

    size_t p = 0;

    eassert(!memcmp(cmds->strs[p].value, "$(", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_MATH_EXPR_START);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "+", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_ADD);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "-", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_SUB);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "*", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_MUL);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "/", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_DIV);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "%", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_MOD);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, "**", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_EXP);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!memcmp(cmds->strs[p].value, ")", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_MATH_EXPR_END);

    eassert(!cmds->strs[p].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_while_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("count=1\nwhile [ $count -lt 3 ]; do echo $count; count=$(count + 1) done");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    if (res.parser_errno) {
        printf("%s\n", res.output.msg);
    }
    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_WHILE);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;
    eassert(stmt->type == LT_NORMAL);
    eassert(cmds->count == 3);

    size_t p = 0;

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(cmds->ops[p++] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!cmds->strs[p].value);
    eassert(!cmds->next);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_WHILE_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(cmds->ops[p++] == OP_LT_A);

    eassert(!memcmp(cmds->strs[p].value, "3", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(!cmds->strs[p].value);
    eassert(!cmds->next);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_WHILE);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, ECHO.value, ECHO.length - 1));
    eassert(cmds->strs[p].length == ECHO.length);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!cmds->strs[p].value);
    eassert(!cmds->next);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_WHILE);
    cmds = stmt->commands;
    eassert(cmds);
    eassert(cmds->op == OP_ASSIGNMENT)
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(!memcmp(cmds->strs[p].value, "=", 2));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_ASSIGNMENT);

    cmds = cmds->next;
    eassert(cmds);
    p = 0;

    eassert(cmds->ops[p++] == OP_MATH_EXPR_START);

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(cmds->ops[p++] == OP_ADD);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    eassert(cmds->ops[p++] == OP_MATH_EXPR_END);

    eassert(!cmds->strs[p].value);

    // jump op back to conditions
    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_WHILE_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(cmds->ops[p] == OP_JUMP);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_WHILE_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, "count", 5));
    eassert(cmds->strs[p].length == 6);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parse_for_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("for ((i = 1; i <= 5; i++)); do echo $i done");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    if (res.parser_errno) {
        printf("%s\n", res.output.msg);
    }
    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_FOR);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;
    size_t p = 0;
    eassert(stmt->type == LT_FOR_INIT);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "=", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "<=", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_LE);

    eassert(!memcmp(cmds->strs[p].value, "5", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR_INCREMENT);
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "++", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_INCREMENT);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR);
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[p].value, "echo", 4));
    eassert(cmds->strs[p].length == 5);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    // jump op back to conditions
    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_FOR_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(cmds->ops[p] == OP_JUMP);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_FOR_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}


void parse_for_each_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto line = Str_Lit("for fruit in apple banana orange; do echo $fruit done");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &scratch_arena);
    auto res = parse(&lexemes, &scratch_arena);

    if (res.parser_errno) {
        printf("%s\n", res.output.msg);
    }
    eassert(!res.parser_errno);
    eassert(res.output.stmts);
    auto stmts = res.output.stmts;
    eassert(stmts->type == ST_FOR);
    eassert(!stmts->is_bg_job);
    eassert(stmts->pipes_count == 1);
    eassert(stmts->redirect_type == RT_NONE);
    eassert(stmts->head);
    eassert(stmts->head->commands);
    auto stmt = stmts->head;
    auto cmds = stmt->commands;
    size_t p = 0;
    eassert(stmt->type == LT_FOR_INIT);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "=", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_ASSIGNMENT);

    eassert(!memcmp(cmds->strs[p].value, "1", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR_CONDITIONS);
    eassert(cmds->count == 3);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "<=", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_LE);

    eassert(!memcmp(cmds->strs[p].value, "5", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_NUM);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR_INCREMENT);
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    eassert(!memcmp(cmds->strs[p].value, "++", 2));
    eassert(cmds->strs[p].length == 3);
    eassert(cmds->ops[p++] == OP_INCREMENT);

    stmt = stmt->right;
    cmds = stmt->commands;
    p = 0;
    eassert(stmt->type == LT_FOR);
    eassert(cmds->count == 2);

    eassert(!memcmp(cmds->strs[p].value, "echo", 4));
    eassert(cmds->strs[p].length == 5);
    eassert(cmds->ops[p++] == OP_CONST);

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    // jump op back to conditions
    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_FOR_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(cmds->ops[p] == OP_JUMP);

    stmt = stmt->right;
    eassert(stmt);
    eassert(stmt->type == LT_FOR_CONDITIONS);
    cmds = stmt->commands;
    eassert(cmds);
    p = 0;

    eassert(!memcmp(cmds->strs[p].value, "i", 1));
    eassert(cmds->strs[p].length == 2);
    eassert(cmds->ops[p++] == OP_VARIABLE);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void parser_tests()
{
    // etest_init(true);
    etest_start();

    etest_run(parse_ls_test);
    etest_run(parse_ls_dash_l_test);

    etest_run(parse_pipe_test);
    etest_run(parse_multiple_pipes_test);
    etest_run(parse_multiple_pipes_multiple_args_test);

    etest_run(parse_background_job_test);
    etest_run(parse_background_job_args_test);

    etest_run(parse_output_redirection_test);
    etest_run(parse_output_redirection_append_test);
    etest_run(parse_input_redirection_test);
    etest_run(parse_input_redirection_append_test);
    etest_run(parse_stderr_redirection_test);
    etest_run(parse_stderr_redirection_append_test);
    etest_run(parse_stdout_and_stderr_redirection_test);
    etest_run(parse_stdout_and_stderr_redirection_append_test);

    etest_run(parse_assignment_test);
    etest_run(parse_assignment_spaces_test);
    etest_run(parse_assignment_spaces_multiple_test);
    etest_run(parse_variable_test);
    etest_run(parse_variable_and_test);

    etest_run(parse_home_test);
    etest_run(parse_home_at_start_test);
    etest_run(parse_glob_star_test);
    etest_run(parse_glob_question_test);
    etest_run(parse_glob_question_midcommand_test);
    etest_run(parse_glob_star_shouldnt_crash);
    etest_run(parse_tilde_home_shouldnt_crash);
    etest_run(parse_glob_question_and_tilde_home_shouldnt_crash);

    etest_run(parse_comment_test);

    etest_run(parse_bool_test);

    etest_run(parse_if_test);
    etest_run(parse_if_eq_test);
    etest_run(parse_if_variable_test);
    etest_run(parse_if_multiple_conditions_test);
    etest_run(parse_if_else_test);
    etest_run(parse_if_elif_test);
    etest_run(parse_if_elif_else_test);
    etest_run(parse_if_elif_multiple_else_test);

    etest_run(parse_math_operators_test);

    etest_run(parse_while_test);
    etest_run(parse_for_test);

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
