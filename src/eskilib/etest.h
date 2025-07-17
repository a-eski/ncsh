/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW_BRIGHT "\033[93m"

#define eassert(condition)                                                                                             \
    if (!(condition)) {                                                                                                \
        printf("%s condition (%s) failed on line %d\n", __func__, #condition, __LINE__);                               \
        etest_failed_internal();                                                                                       \
        return;                                                                                                        \
    }

#define etest_run(function) etest_run_internal(#function, function);

#define etest_run_tester(function_name, code)                                                                          \
    {                                                                                                                  \
        test_failed = false;                                                                                           \
        printf("%s STARTED.\n", function_name);                                                                        \
                                                                                                                       \
        {                                                                                                              \
            code                                                                                                       \
        }                                                                                                              \
                                                                                                                       \
        if (test_failed) {                                                                                             \
            ++tests_failed;                                                                                            \
            printf(RED "%s FAILED.\n" RESET, function_name);                                                           \
        }                                                                                                              \
        else {                                                                                                         \
            ++tests_passed;                                                                                            \
            printf(GREEN "%s PASSED.\n" RESET, function_name);                                                         \
        }                                                                                                              \
    }

// Call at the start of a series of tests
#define etest_start() etest_start_internal(__FILE__);

// Call at the end of a series of tests
#define etest_finish() etest_finish_internal(__FILE__);

static bool test_failed;

static int tests_failed;
static int tests_passed;

static void etest_run_internal(char* function_name, void (*function)(void))
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

[[maybe_unused]]
static void etest_failed_internal(void)
{
    test_failed = true;
}

static void etest_start_internal(char* file)
{
    test_failed = 0;
    tests_passed = 0;
    printf(YELLOW_BRIGHT "Starting tests for %s\n" RESET, file);
}

static void etest_finish_internal(char* file)
{
    printf(YELLOW_BRIGHT "Finished tests for %s: %d tests passed, %d tests failed.\n" RESET, file, tests_passed,
           tests_failed);
}
