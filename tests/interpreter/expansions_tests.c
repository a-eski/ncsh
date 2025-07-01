#include <stdlib.h>

#include "../lib/arena_test_helper.h"
#include "../lib/shell_test_helper.h"
#include "../../src/eskilib/etest.h"
#include "../../src/env.h"
#include "../../src/interpreter/expansions.h"
#include "../../src/interpreter/lexemes.h"

void expansion_home_test()
{
    ARENA_TEST_SETUP;
    Arena a = arena;
    Str home = {0};
    env_home_get(&home, &a);

    Lexemes lexemes = {.count = 1};
    lexemes_init(&lexemes, &a);
    lexemes.vals[0] = arena_malloc(&a, 2, char);
    lexemes.vals[0][0] = '~';
    lexemes.lens[0] = 2;
    lexemes.ops[0] = OP_HOME_EXPANSION;

    expansion_home(&lexemes, 0, &a);

    eassert(!memcmp(lexemes.vals[0], home.value, home.length - 1));
    eassert(lexemes.lens[0] == home.length);
    eassert(lexemes.ops[0] == OP_CONSTANT);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_path_test()
{
    ARENA_TEST_SETUP;
    Arena a = arena;
    Str path = env_path_get();

    Commands* cmds = commands_alloc(&a);

    expansion_variable("$PATH", sizeof("$PATH"), cmds, NULL, &a);

    eassert(!memcmp(cmds->vals[0], path.value, path.length - 1));
    cmds->lens[0] = path.length;
    cmds->ops[0] = OP_CONSTANT;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_home_test()
{
    ARENA_TEST_SETUP;
    Arena a = arena;
    Str home = {0};
    env_home_get(&home, &a);

    Commands* cmds = commands_alloc(&a);

    expansion_variable("$HOME", sizeof("$HOME"), cmds, NULL, &a);

    eassert(!memcmp(cmds->vals[0], home.value, home.length - 1));
    cmds->lens[0] = home.length;
    cmds->ops[0] = OP_CONSTANT;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

void expansion_variable_test()
{
    ARENA_TEST_SETUP;
    Arena a = arena;
    Shell shell = {0};
    shell_init(&shell, &a);
    vars_set("VAL", &Str_New_Literal("hello"), &a, &shell.vars);

    Commands* cmds = commands_alloc(&a);

    expansion_variable("$VAL", sizeof("$VAL"), cmds, &shell, &a);

    eassert(!memcmp(cmds->vals[0], "hello", sizeof("hello") - 1));
    cmds->lens[0] = sizeof("hello");
    cmds->ops[0] = OP_CONSTANT;
    eassert(cmds->pos == 1);

    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(expansion_home_test);
    etest_run(expansion_variable_path_test);
    etest_run(expansion_variable_home_test);
    etest_run(expansion_variable_test);

    etest_finish();

    return EXIT_SUCCESS;
}
