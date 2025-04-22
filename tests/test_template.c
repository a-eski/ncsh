/* test_template.c: a template for creating new tests for ncsh. */

#include <stdlib.h>

#include "../src/eskilib/etest.h"

void template_example_test()
{
}

int main()
{
    etest_start();

    etest_run(template_example_test);

    etest_finish();

    return EXIT_SUCCESS;
}
