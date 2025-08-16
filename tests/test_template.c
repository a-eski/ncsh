/* test_template.c: a template for creating new tests for ncsh. */

#include "etest.h"

void template_example_test()
{
}

void template_tests()
{
    etest_start();

    etest_run(template_example_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    return 0;
}
#endif /* ifndef TEST_ALL */
