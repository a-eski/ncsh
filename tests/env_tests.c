#include <stdlib.h>

#include "etest.h"
#include "../src/env.h"
#include "lib/arena_test_helper.h"

static char** envp_ptr;

void env_add_or_get_home_test()
{
    ARENA_TEST_SETUP;

    char* env_home = getenv("HOME");
    size_t env_home_len = strlen(env_home) + 1;
    Shell s = {0};
    env_new(&s, envp_ptr, &arena);
    Str* home = env_home_get(s.env);

    eassert(home);
    eassert(home->value);
    eassert(home->length == env_home_len);
    eassert(!memcmp(home->value, env_home, home->length - 1));

    ARENA_TEST_TEARDOWN;
}

void env_add_or_get_path_test()
{
    ARENA_TEST_SETUP;

    char* env_path = getenv("PATH");
    size_t env_path_len = strlen(env_path) + 1;

    Shell s = {0};
    env_new(&s, envp_ptr, &arena);
    Str* path = env_add_or_get(s.env, Str_New_Literal(NCSH_PATH_VAL));

    eassert(path);
    eassert(path->value);
    eassert(path->length == env_path_len);
    eassert(!memcmp(path->value, env_path, path->length - 1));

    ARENA_TEST_TEARDOWN;
}

void env_add_or_get_user_test()
{
    ARENA_TEST_SETUP;

    char* env_usr = getenv("USER");
    size_t env_usr_len = strlen(env_usr) + 1;

    Shell s = {0};
    env_new(&s, envp_ptr, &arena);
    Str* usr = env_add_or_get(s.env, Str_New_Literal(NCSH_USER_VAL));

    eassert(usr);
    eassert(usr->value);
    eassert(usr->length == env_usr_len);
    eassert(!memcmp(usr->value, env_usr, usr->length - 1));

    ARENA_TEST_TEARDOWN;
}

void env_tests()
{
    etest_start();

    etest_run(env_add_or_get_home_test);
    etest_run(env_add_or_get_path_test);
    etest_run(env_add_or_get_user_test);

    etest_finish();
}

#ifndef TEST_ALL
int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv,
         char** envp)
{
    envp_ptr = envp;

    env_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
