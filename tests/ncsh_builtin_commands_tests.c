#include <stdio.h>
#include <stdlib.h>

#include "../ncsh_builtin_commands.h"
#include "../eskilib/eskilib_test.h"
#include "../eskilib/eskilib_string.h"
#include "ncsh_builtin_commands_tests.h"

void ncsh_history_malloc_test(void) {
	ncsh_history_malloc();

	struct eskilib_String result = ncsh_history_get(0);

	eskilib_assert(result.length == 0);
	eskilib_assert(result.value == NULL);

	ncsh_history_free();
}

void ncsh_history_add_uninitialized_test(void) {
	#define NDEBUG
	#include <assert.h>
	ncsh_history_add("ls", 3); // should not crash
	#undef NDEBUG
}

void ncsh_history_add_test(void) {
	ncsh_history_malloc();

	ncsh_history_add("ls", 3);

	struct eskilib_String result = ncsh_history_get(0);

	eskilib_assert(result.length == 3);
	eskilib_assert(eskilib_string_equals(result.value, "ls", 3));

	ncsh_history_free();
}

void ncsh_history_add_multiple_test(void) {
	char* ls = "ls";
	char* ls_pipe_sort = "ls | sort";

	ncsh_history_malloc();

	ncsh_history_add(ls, 3);
	ncsh_history_add(ls_pipe_sort, 10);

	struct eskilib_String result_one = ncsh_history_get(0);

	eskilib_assert(result_one.length == 10);
	eskilib_assert(eskilib_string_equals(result_one.value, ls_pipe_sort, result_one.length));

	struct eskilib_String result_two = ncsh_history_get(1);

	eskilib_assert(result_two.length == 3);
	eskilib_assert(eskilib_string_equals(result_two.value, ls, result_two.length));

	ncsh_history_free();
}

void ncsh_history_add_multiple_before_get_test(void) {
	char* ls = "ls";
	char* ls_dash_l = "ls -l";
	char* ls_pipe_sort = "ls | sort";

	ncsh_history_malloc();

	ncsh_history_add(ls, 3);
	ncsh_history_add(ls_dash_l, 5);
	ncsh_history_add(ls_pipe_sort, 10);

	struct eskilib_String result_one = ncsh_history_get(0);
	eskilib_assert(result_one.length == 10);
	eskilib_assert(eskilib_string_equals(result_one.value, ls_pipe_sort, result_one.length));

	struct eskilib_String result_two = ncsh_history_get(1);
	eskilib_assert(result_two.length == 5);
	eskilib_assert(eskilib_string_equals(result_two.value, ls_dash_l, result_two.length));

	struct eskilib_String result_three = ncsh_history_get(0);
	eskilib_assert(result_three.length == 10);
	eskilib_assert(eskilib_string_equals(result_three.value, ls_pipe_sort, result_three.length));

	struct eskilib_String result_four = ncsh_history_get(1);
	eskilib_assert(result_four.length == 5);
	eskilib_assert(eskilib_string_equals(result_four.value, ls_pipe_sort, result_four.length));

	struct eskilib_String result_five = ncsh_history_get(2);
	eskilib_assert(result_five.length == 3);
	eskilib_assert(eskilib_string_equals(result_five.value, ls, result_five.length));

	ncsh_history_free();
}

void ncsh_history_get_out_of_range_test(void) {
	char* ls = "ls";
	char* ls_pipe_sort = "ls | sort";

	ncsh_history_malloc();

	ncsh_history_add(ls, 3);
	ncsh_history_add(ls_pipe_sort, 10);

	struct eskilib_String result_one = ncsh_history_get(0);

	eskilib_assert(result_one.length == 10);
	eskilib_assert(eskilib_string_equals(result_one.value, ls_pipe_sort, result_one.length));

	struct eskilib_String result_two = ncsh_history_get(1);

	eskilib_assert(result_two.length == 3);
	eskilib_assert(eskilib_string_equals(result_two.value, ls, result_two.length));

	struct eskilib_String result_three = ncsh_history_get(2);

	eskilib_assert(result_three.length == 0);
	eskilib_assert(result_three.value == NULL);

	ncsh_history_free();
}

void ncsh_history_get_multiple_test(void) {
	char* ls = "ls";
	char* ls_pipe_sort = "ls | sort";

	ncsh_history_malloc();

	ncsh_history_add(ls, 3);
	ncsh_history_add(ls_pipe_sort, 10);

	struct eskilib_String result_one = ncsh_history_get(0);

	eskilib_assert(result_one.length == 10);
	eskilib_assert(eskilib_string_equals(result_one.value, ls_pipe_sort, result_one.length));

	struct eskilib_String result_two = ncsh_history_get(1);

	eskilib_assert(result_two.length == 3);
	eskilib_assert(eskilib_string_equals(result_two.value, ls, result_two.length));

	struct eskilib_String result_three = ncsh_history_get(0);

	eskilib_assert(result_three.length == 10);
	eskilib_assert(eskilib_string_equals(result_three.value, ls_pipe_sort, result_three.length));

	ncsh_history_free();
}

void ncsh_builtin_commands_tests(void) {
	eskilib_test_run("ncsh_history_malloc_test", ncsh_history_malloc_test);
	eskilib_test_run("ncsh_history_add_test", ncsh_history_add_test);
	eskilib_test_run("ncsh_history_add_uninitialized_test", ncsh_history_add_uninitialized_test);
	eskilib_test_run("ncsh_history_add_multiple_test", ncsh_history_add_multiple_test);
	eskilib_test_run("ncsh_history_get_out_of_range_test", ncsh_history_get_out_of_range_test);
	eskilib_test_run("ncsh_history_get_multiple_test", ncsh_history_get_multiple_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_builtin_commands_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
