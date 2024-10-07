#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../eskilib/eskilib_test.h"
#include "../eskilib/eskilib_string.h"

#define NCSH_HISTORY_TEST
#include "../ncsh_builtin_commands.h"

void ncsh_history_load_file_not_exists_test() {
	remove(NCSH_HISTORY_FILE);

	ncsh_history_load();

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	fclose(file);
}

void ncsh_history_load_file_exists_test() {
	ncsh_history_load();

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	fclose(file);
}

void ncsh_history_load_entries_test() {
	ncsh_history_malloc();
	ncsh_history_load();

	// struct eskilib_String result = ncsh_history_get(0);
	// eskilib_assert(eskilib_string_equals(result.value, "ls\n", result.length));

	ncsh_history_free();
}

void ncsh_history_save_adds_to_file() {
	ncsh_history_malloc();
	ncsh_history_load();

	ncsh_history_add("ls\0", 3);

	ncsh_history_save();

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	int max = MAX_INPUT;
	char buffer[max];
	fgets(buffer, sizeof(buffer), file);
	eskilib_assert(eskilib_string_equals(buffer, "ls\n", max));
	fclose(file);
	ncsh_history_free();
}

void ncsh_history_save_adds_multiple_to_file() {
	ncsh_history_malloc();
	ncsh_history_load();

	char* existing_command = "ls\n";
	ncsh_history_add("ls | sort\0", 10);
	ncsh_history_add("echo hello\0", 11);

	ncsh_history_save();

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	int max = MAX_INPUT;
	char buffer[max];
	for (uint_fast8_t i = 0; fgets(buffer, sizeof(buffer), file); i++) {
		if (i == 0)
			eskilib_assert(eskilib_string_equals(buffer, existing_command, max));
		if (i == 1)
			eskilib_assert(eskilib_string_equals(buffer, "ls | sort\n", max));
		if (i == 2)
			eskilib_assert(eskilib_string_equals(buffer, "echo hello\n", max));
	}

	fclose(file);
	ncsh_history_free();
}

void ncsh_history_file_tests(void) {
	eskilib_test_run("ncsh_history_load_file_not_exists_test", ncsh_history_load_file_not_exists_test);
	eskilib_test_run("ncsh_history_load_file_exists_test", ncsh_history_load_file_exists_test);
	eskilib_test_run("ncsh_history_save_adds_to_file", ncsh_history_save_adds_to_file);
	eskilib_test_run("ncsh_history_save_adds_multiple_to_file", ncsh_history_save_adds_multiple_to_file);
	eskilib_test_run("ncsh_history_load_entries_test", ncsh_history_load_entries_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_history_file_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
