/* Copyright eskilib by Alex Eski 2024 */

#include <stdio.h>

#include "eskilib_test.h"
#include "eskilib_colors.h"

static bool test_failed;

static int tests_failed;
static int tests_passed;

void eskilib_test_run(const char* function_name, void (*function)(void))
{
	test_failed = false;
	printf("%s started.\n", function_name);

	function();

	if (test_failed) {
		++tests_failed;
		printf(RED "%s failed.\n" RESET, function_name);
	}
	else {
		++tests_passed;
		printf(GREEN "%s passed.\n" RESET , function_name);
	}
}

void eskilib_test_failed_internal(void) {
	test_failed = true;
}

void eskilib_test_start_internal(char* file) {
	test_failed = 0;
	tests_passed = 0;
	printf(YELLOW_BRIGHT "Starting tests for %s\n" RESET, file);
}

void eskilib_test_finish(void) {
	printf(YELLOW_BRIGHT "Tests finished: %d tests passed, %d tests failed.\n" RESET, tests_passed, tests_failed);
}

