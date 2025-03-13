#include <stdlib.h>

#include "../src/eskilib/eskilib_test.h"

void ncsh_template_example_test(void)
{
}

void ncsh_template_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(ncsh_template_example_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_template_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
