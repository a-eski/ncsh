#include <stdlib.h>

#include "../lib/arena_test_helper.h"
#include "../lib/shell_test_helper.h"
#include "../etest.h"
#include "../../src/env.h"
#include "../../src/interpreter/expansions.h"
#include "../../src/interpreter/lexemes.h"

static char** envp_ptr;

void expansion_home_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    char* home = getenv(NCSH_HOME_VAL);
    size_t home_len = strlen(home) + 1;
    printf("home %s\n", home);

    Lexemes lexemes = {0};
    lexemes_init(&lexemes, &a);
    lexemes.count = 1;
    lexemes.strs[0].value = arena_malloc(&a, 2, char);
    lexemes.strs[0].value[0] = '~';
    lexemes.strs[0].length = 2;
    lexemes.ops[0] = OP_HOME_EXPANSION;

    expansion_home(&(Parser_Data){.lexemes = &lexemes, .sh = &shell, .s = &a}, 0);

    printf("%s\n", lexemes.strs[0].value);
    eassert(!memcmp(lexemes.strs[0].value, home, home_len - 1));
    eassert(lexemes.strs[0].length == home_len);
    eassert(lexemes.ops[0] == OP_CONST);

    ARENA_TEST_TEARDOWN;
}

void expansion_home_suffix_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    Str home = Str_Get(getenv(NCSH_HOME_VAL));
    Str* expected = estrcat(&home, &Str_New_Literal("/snap"), &a);

    #define V "~/snap"
    Lexemes lexemes = {0};
    lexemes_init(&lexemes, &a);
    lexemes.count = 1;
    lexemes.strs[0].value = arena_malloc(&a, sizeof(V), char);
    memcpy(lexemes.strs[0].value, V, sizeof(V) - 1);
    lexemes.strs[0].length = sizeof(V);
    lexemes.ops[0] = OP_HOME_EXPANSION;
    #undef V

    expansion_home(&(Parser_Data){.lexemes = &lexemes, .sh = &shell, .s = &a}, 0);

    eassert(!memcmp(lexemes.strs[0].value, expected->value, expected->length - 1));
    eassert(lexemes.strs[0].length == expected->length);
    eassert(lexemes.ops[0] == OP_CONST);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_path_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    char* path = getenv(NCSH_PATH_VAL);
    size_t path_len = strlen(path) + 1;
    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("$PATH"));

    eassert(cmds->strs);
    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, path, path_len - 1));
    eassert(cmds->strs[0].length == path_len);
    eassert(cmds->ops[0] == OP_CONST);
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_path_no_dollar_sign_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    char* path = getenv(NCSH_PATH_VAL);
    size_t path_len = strlen(path) + 1;

    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("PATH"));

    eassert(cmds->strs);
    eassert(cmds->strs[0].value);
    eassert(!memcmp(cmds->strs[0].value, path, path_len - 1));
    eassert(cmds->strs[0].length == path_len);
    eassert(cmds->ops[0] = OP_CONST);
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_home_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    Str home = *env_home_get(shell.env);
    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("$HOME"));

    eassert(!memcmp(cmds->strs[0].value, home.value, home.length - 1));
    cmds->strs[0].length = home.length;
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    *env_add_or_get(shell.env, Str_New_Literal("VAL")) = Str_New_Literal("hello");
    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("$VAL"));

    eassert(!memcmp(cmds->strs[0].value, "hello", sizeof("hello") - 1));
    cmds->strs[0].length = sizeof("hello");
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_no_dollar_sign_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    *env_add_or_get(shell.env, Str_New_Literal("VAL")) = Str_New_Literal("hello");

    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("VAL"));

    eassert(!memcmp(cmds->strs[0].value, "hello", sizeof("hello") - 1));
    cmds->strs[0].length = sizeof("hello");
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_with_spaces_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    Str v = Str_New_Literal("VAL");
    *env_add_or_get(shell.env, v) = Str_New_Literal("hello      there , 1");

    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("$VAL"));

    eassert(!memcmp(cmds->strs[0].value, v.value, v.length - 1));
    cmds->strs[0].length = v.length;
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_number_test()
{
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &a, envp_ptr);
    Str v = Str_New_Literal("1");
    *env_add_or_get(shell.env, v) = Str_New_Literal("1");

    Commands* cmds = cmds_alloc(&a);

    expansion_variable(&(Parser_Data){.cur_cmds = cmds, .sh = &shell, .s = &a}, &Str_New_Literal("$VAL"));

    eassert(!memcmp(cmds->strs[0].value, v.value, v.length - 1));
    cmds->strs[0].length = v.length;
    cmds->ops[0] = OP_CONST;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_tests()
{
    etest_start();

    // TODO: fix/debug these tests
    // etest_run(expansion_home_test);
    // etest_run(expansion_home_suffix_test);
    etest_run(expansion_variable_path_test);
    etest_run(expansion_variable_path_no_dollar_sign_test);
    etest_run(expansion_variable_home_test);
    etest_run(expansion_variable_test);
    // etest_run(expansion_variable_with_spaces_test);
    etest_run(expansion_variable_no_dollar_sign_test);
    // etest_run(expansion_variable_number_test);

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
