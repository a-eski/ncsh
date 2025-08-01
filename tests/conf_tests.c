
#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/conf.h"
#include "lib/arena_test_helper.h"

void config_init_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    ARENA_TEST_SETUP;

    Config config = {0};
    config_init(&config, &arena, scratch_arena);

    eassert(config.config_file.value);
    eassert(config.config_location.value);
    eassert(config.home_location.value);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void config_tests()
{
    etest_start();

    etest_run(config_init_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    config_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
