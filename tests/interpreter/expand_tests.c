#include <stdlib.h>

#include "../lib/arena_test_helper.h"
#include "../lib/shell_test_helper.h"
#include "../etest.h"
#include "../../src/env.h"
#include "../../src/interpreter/expand.h"
#include "../../src/interpreter/parse.h"
#include "../../src/interpreter/lex.h"

static char** envp_ptr;

void expand_home_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    auto home = Str_Get(getenv(NCSH_HOME_VAL));

    auto line = Str_Lit("ls ~");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    eassert(!memcmp(cmds->strs[1].value, home.value, home.length - 1));
    eassert(cmds->strs[1].length == home.length);
    eassert(cmds->ops[1] == OP_CONST);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_home_suffix_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    auto home = Str_Get(getenv(NCSH_HOME_VAL));

    auto line = Str_Lit("ls ~/snap");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto parse_res = parse(&lexemes, &s);

    expand(parse_res.output.stmts, &shell, &s);

    auto res = estrcat(&home, &Str_Lit("/snap"), &s);

    auto cmds = parse_res.output.stmts->head->commands;
    eassert(!memcmp(cmds->strs[1].value, res->value, res->length - 1));
    eassert(cmds->strs[1].length == res->length);
    eassert(cmds->ops[1] == OP_CONST);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_glob_star_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);

    auto line = Str_Lit("ls *.md");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    auto compile = Str_Lit("COMPILE.md");
    eassert(!memcmp(cmds->strs[1].value, compile.value, compile.length - 1));
    eassert(cmds->strs[1].length == compile.length);
    eassert(cmds->ops[1] == OP_CONST);

    auto notes = Str_Lit("NOTES.md");
    eassert(!memcmp(cmds->strs[2].value, notes.value, notes.length - 1));
    eassert(cmds->strs[2].length == notes.length);
    eassert(cmds->ops[2] == OP_CONST);

    auto readme = Str_Lit("README.md");
    eassert(!memcmp(cmds->strs[3].value, readme.value, readme.length - 1));
    eassert(cmds->strs[3].length == readme.length);
    eassert(cmds->ops[3] == OP_CONST);

    eassert(!cmds->strs[4].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_glob_question_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);

    auto line = Str_Lit("ls src/z/?.c");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    auto z = Str_Lit("src/z/z.c");
    eassert(!memcmp(cmds->strs[1].value, z.value, z.length - 1));
    eassert(cmds->strs[1].length == z.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_var_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    *env_add_or_get(shell.env, Str_Lit("VAL")) = Str_Lit("1");

    auto line = Str_Lit("echo $VAL");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    auto one = Str_Lit("1");
    eassert(!memcmp(cmds->strs[1].value, one.value, one.length - 1));
    eassert(cmds->strs[1].length == one.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_var_path_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);

    auto line = Str_Lit("echo $PATH");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    auto path = *env_add_or_get(shell.env, Str_Lit("PATH"));
    eassert(!memcmp(cmds->strs[1].value, path.value, path.length - 1));
    eassert(cmds->strs[1].length == path.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void expand_var_home_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);

    auto line = Str_Lit("echo $HOME");

    Lexemes lexemes = {0};
    lex(line, &lexemes, &s);
    auto res = parse(&lexemes, &s);

    expand(res.output.stmts, &shell, &s);

    auto cmds = res.output.stmts->head->commands;
    auto home = *env_home_get(shell.env);
    eassert(!memcmp(cmds->strs[1].value, home.value, home.length - 1));
    eassert(cmds->strs[1].length == home.length);
    eassert(cmds->ops[1] == OP_CONST);

    eassert(!cmds->strs[2].value);
    eassert(!cmds->next);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

/*void expand_var_whitespace_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    Str v = Str_Lit("VAL");
    *env_add_or_get(shell.env, v) = Str_Lit("hello      there , 1");

    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_Lit("$VAL"));

    eassert(!memcmp(cmds->strs[0].value, v.value, v.length - 1));
    cmds->strs[0].length = v.length;
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}*/

void expansion_tests()
{
    etest_start();

    etest_run(expand_home_test);
    etest_run(expand_home_suffix_test);
    etest_run(expand_glob_star_test);
    etest_run(expand_glob_question_test);
    etest_run(expand_var_test);
    etest_run(expand_var_path_test);
    etest_run(expand_var_home_test);
    // etest_run(expand_var_whitespace_test);

    etest_finish();
}

#ifndef TEST_ALL
int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv,
         char** envp)
{
    envp_ptr = envp;

    expansion_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
