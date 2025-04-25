/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define eassert(condition)                                                                                             \
    if (!(condition)) {                                                                                                \
        printf("%s condition (%s) failed on line %d\n", __func__, #condition, __LINE__);                               \
        etest_failed_internal();                                                                                       \
        return;                                                                                                        \
    }

#define etest_run(function) etest_run_internal(#function, function);

#define etest_run_tester(function_name, code)                       \
{                                                                   \
    test_failed = false;                                            \
    printf("%s STARTED.\n", function_name);                         \
                                                                    \
    {                                                               \
        code                                                        \
    }                                                               \
                                                                    \
    if (test_failed) {                                              \
        ++tests_failed;                                             \
        printf(RED "%s FAILED.\n" RESET, function_name);            \
    }                                                               \
    else {                                                          \
        ++tests_passed;                                             \
        printf(GREEN "%s PASSED.\n" RESET, function_name);          \
    }                                                               \
}

extern void etest_run_internal(const char* function_name, void (*function)(void));

// private API: do not need to use this manually, used with eassert to mark test failed when assertion does not
// pass.
extern void etest_failed_internal(void);

// private API: is called at the start of a series of tests.
extern void etest_start_internal(char*);

// Call at the start of a series of tests
#define etest_start() etest_start_internal(__FILE__);

// private API: is called at the end of a series of tests for a summary.
extern void etest_finish_internal(char* file);

// Call at the end of a series of tests
#define etest_finish() etest_finish_internal(__FILE__);
