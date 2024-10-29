#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "../eskilib_test.h"
#include "../eskilib_string.h"

void eskilib_string_contains_unsafe_true_test(void) {
	bool result = eskilib_string_contains_unsafe("ls -l", "ls");

	eskilib_assert(result == true);
}

void eskilib_string_contains_unsafe_false_test(void) {
	bool result = eskilib_string_contains_unsafe("ls -l", "nvim");

	eskilib_assert(result == false);
}

void eskilib_string_contains_true_test(void) {
	struct eskilib_String string = { .value = "ls -l", .length = 6 };
	struct eskilib_String substring = { .value = "ls", .length = 3 };

	bool result = eskilib_string_contains(string, substring);

	eskilib_assert(result == true);
}

void eskilib_string_contains_false_test(void) {
	struct eskilib_String string = { .value = "ls -l", .length = 6 };
	struct eskilib_String substring = { .value = "nvim", .length = 5 };

	bool result = eskilib_string_contains(string, substring);

	eskilib_assert(result == false);
}

void eskilib_string_contains_end_of_string_test(void) {
	struct eskilib_String string = { .value = "C:\\Users\\Alex\\source\\repos\\PersonalRepos\\shells\\ncsh", .length = 53};
	struct eskilib_String substring = { .value = "ncsh", .length = 5 };

	bool result_unsafe = eskilib_string_contains_unsafe(string.value, substring.value);

	eskilib_assert(result_unsafe == true);

	bool result = eskilib_string_contains(string, substring);

	eskilib_assert(result == true);

}

void eskilib_string_tests(void) {
	eskilib_test_run("eskilib_string_contains_unsafe_false_test", eskilib_string_contains_unsafe_false_test);
	eskilib_test_run("eskilib_string_contains_unsafe_true_test", eskilib_string_contains_unsafe_true_test);
	eskilib_test_run("eskilib_string_contains_false_test", eskilib_string_contains_false_test);
	eskilib_test_run("eskilib_string_contains_true_test", eskilib_string_contains_true_test);
	eskilib_test_run("eskilib_string_contains_end_of_string_test", eskilib_string_contains_end_of_string_test);
}

#ifndef eskilib_TEST_ALL
int main(void) {
	eskilib_string_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */

