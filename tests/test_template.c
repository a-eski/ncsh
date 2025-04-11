#include <stdlib.h>

#include "../src/eskilib/etest.h"

void template_example_test(void)
{
}

void template_tests(void)
{
    etest_start();

    etest_run(template_example_test);

    etest_finish();
}

int main(void)
{
    template_tests();

    return EXIT_SUCCESS;
}
