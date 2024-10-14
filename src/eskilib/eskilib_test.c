#include <stdio.h>

#include "eskilib_test.h"
#include "eskilib_colors.h"

static bool test_failed;

void eskilib_test_run(const char* function_name, void (*function)(void))
{
	test_failed = false;
	printf("\x1B[37m%s started.\n", function_name);

	function();

	if (test_failed)
		printf(RED "%s failed.\n" RESET, function_name);
	else
		printf(GREEN "%s passed.\n" RESET , function_name);
}

void eskilib_test_failed(void) {
	test_failed = true;
}

