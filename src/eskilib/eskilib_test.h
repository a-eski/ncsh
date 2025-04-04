/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

#define eskilib_assert(condition)                                                                                      \
    if (!(condition)) {                                                                                                \
        printf("%s condition (%s) failed on line %d\n", __func__, #condition, __LINE__);                               \
        eskilib_test_failed_internal();                                                                                \
        return;                                                                                                        \
    }

#define eskilib_test_run(function) eskilib_test_run_internal(#function, function);

extern void eskilib_test_run_internal(const char* function_name, void (*function)(void));

// private API: do not need to use this manually, used with eskilib_assert to mark test failed when assertion does not
// pass.
extern void eskilib_test_failed_internal(void);

// private API: is called at the start of a series of tests.
extern void eskilib_test_start_internal(char*);

// Call at the start of a series of tests
#define eskilib_test_start() eskilib_test_start_internal(__FILE__);

// private API: is called at the end of a series of tests for a summary.
extern void eskilib_test_finish_internal(char* file);

// Call at the end of a series of tests
#define eskilib_test_finish() eskilib_test_finish_internal(__FILE__);
