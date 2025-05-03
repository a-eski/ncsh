/* Copyright eskilib by Alex Eski 2024 */

#include <stdio.h>

#include "ecolors.h"
#include "etest.h"

bool test_failed;

int tests_failed;
int tests_passed;

void etest_run_internal(char* function_name, void (*function)(void))
{
    test_failed = false;
    printf("%s STARTED.\n", function_name);

    function();

    if (test_failed) {
        ++tests_failed;
        printf(RED "%s FAILED.\n" RESET, function_name);
    }
    else {
        ++tests_passed;
        printf(GREEN "%s PASSED.\n" RESET, function_name);
    }
}

void etest_failed_internal(void)
{
    test_failed = true;
}

void etest_start_internal(char* file)
{
    test_failed = 0;
    tests_passed = 0;
    printf(YELLOW_BRIGHT "Starting tests for %s\n" RESET, file);
}

void etest_finish_internal(char* file)
{
    printf(YELLOW_BRIGHT "Finished tests for %s: %d tests passed, %d tests failed.\n" RESET, file, tests_passed,
           tests_failed);
}
