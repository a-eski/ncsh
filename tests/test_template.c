#include <stdlib.h>

#include "../src/eskilib/eskilib_test.h"

void template_example_test(void)
{
}

void template_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(template_example_test);

    eskilib_test_finish();
}

#ifndef TEST_ALL
int main(void)
{
    template_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
