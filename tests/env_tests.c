#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/env.h"
#include "lib/arena_test_helper.h"

void env_home_get_test()
{
    ARENA_TEST_SETUP;

    char* env_home = getenv("HOME");
    size_t env_home_len = strlen(env_home) + 1;
    Str home = {0};
    env_home_get(&home, &arena);

    eassert(home.value && *home.value);
    eassert(home.length == env_home_len);
    eassert(!memcmp(home.value, env_home, home.length - 1));

    ARENA_TEST_TEARDOWN;
}

void env_path_get_test()
{
    char* env_path = getenv("PATH");
    size_t env_path_len = strlen(env_path) + 1;

    Str path = env_path_get();

    eassert(path.value && *path.value);
    eassert(path.length == env_path_len);
    eassert(!memcmp(path.value, env_path, path.length - 1));
}

int main()
{
    etest_start();

    etest_run(env_home_get_test);
    etest_run(env_path_get_test);

    etest_finish();

    return EXIT_SUCCESS;
}
