
#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/config.h"
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

int main()
{
    etest_start();

    etest_run(config_init_test);

    etest_finish();

    return EXIT_SUCCESS;
}
