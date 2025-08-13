
#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/conf.h"
#include "lib/arena_test_helper.h"
#include "lib/shell_test_helper.h"

static char** envp_ptr;

void conf_init_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    ARENA_TEST_SETUP;

    Shell shell = {0};
    shell_init(&shell, &arena, envp_ptr);
    conf_init(&shell);

    eassert(shell.config.file.value);
    eassert(shell.config.location.value);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
}

void conf_tests()
{
    etest_start();

    etest_run(conf_init_test);

    etest_finish();
}

#ifndef TEST_ALL
int main([[maybe_unused]] int argc,
         [[maybe_unused]] char** argv,
         char** envp)
{
    envp_ptr = envp;

    conf_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
