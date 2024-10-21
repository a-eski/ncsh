#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../eskilib/eskilib_test.h"
#include "../eskilib/eskilib_string.h"

#define NCSH_HISTORY_TEST
#include "../ncsh_history.h"

void ncsh_history_load_file_not_exists_test(void) {
	remove(NCSH_HISTORY_FILE);

	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	fclose(file);

	ncsh_history_free(&history);
}

void ncsh_history_load_file_exists_test(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	fclose(file);

	ncsh_history_free(&history);
}

void ncsh_history_get_empty_file_test(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	struct eskilib_String entry = ncsh_history_get(0, &history);
	eskilib_assert(entry.length == 0);

	ncsh_history_free(&history);
}

void ncsh_history_save_adds_to_file(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	result = ncsh_history_add("ls\0", 3, &history);
	eskilib_assert(result == N_SUCCESS);

	result = ncsh_history_save(&history);
	eskilib_assert(result == N_SUCCESS);

	ncsh_history_free(&history);

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	int max = MAX_INPUT;
	char buffer[max];
	fgets(buffer, sizeof(buffer), file);
	eskilib_assert(eskilib_string_equals(buffer, "ls\n", max));
	fclose(file);
}

void ncsh_history_save_adds_multiple_to_file(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	char* existing_command = "ls\n";
	result = ncsh_history_add("ls | sort\0", 10, &history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_add("echo hello\0", 11, &history);
	eskilib_assert(result == N_SUCCESS);

	result = ncsh_history_save(&history);
	eskilib_assert(result == N_SUCCESS);
	ncsh_history_free(&history);

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	eskilib_assert(file != NULL);
	const int max = MAX_INPUT;
	char buffer[max];
	for (uint_fast8_t i = 0; fgets(buffer, sizeof(buffer), file) != NULL; i++) {
		if (i == 0) {
			eskilib_assert(eskilib_string_equals(buffer, existing_command, max));
		}
		else if (i == 1) {
			eskilib_assert(eskilib_string_equals(buffer, "ls | sort\n", max));
		}
		if (i == 2) {
			eskilib_assert(eskilib_string_equals(buffer, "echo hello\n", max));
		}
		else {
			break;
		}
	}

	fclose(file);
}

void ncsh_history_load_and_get_entries_test(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	struct eskilib_String entry = ncsh_history_get(0, &history);
	eskilib_assert(entry.length == 11);
	eskilib_assert(eskilib_string_equals(entry.value, "echo hello\0", entry.length));

	entry = ncsh_history_get(1, &history);
	eskilib_assert(entry.length == 10);
	eskilib_assert(eskilib_string_equals(entry.value, "ls | sort\0", entry.length));

	entry = ncsh_history_get(2, &history);
	eskilib_assert(entry.length == 3);
	eskilib_assert(eskilib_string_equals(entry.value, "ls\0", entry.length));

	ncsh_history_free(&history);
}

void ncsh_history_load_and_get_entries_then_add_entries_test(void) {
	enum ncsh_Result result;
	struct ncsh_History history = {0};
	result = ncsh_history_malloc(&history);
	eskilib_assert(result == N_SUCCESS);
	result = ncsh_history_load(&history);
	eskilib_assert(result == N_SUCCESS);

	struct eskilib_String entry = ncsh_history_get(0, &history);
	eskilib_assert(entry.length == 11);
	eskilib_assert(eskilib_string_equals(entry.value, "echo hello\0", entry.length));

	entry = ncsh_history_get(1, &history);
	eskilib_assert(entry.length == 10);
	eskilib_assert(eskilib_string_equals(entry.value, "ls | sort\0", entry.length));

	entry = ncsh_history_get(2, &history);
	eskilib_assert(entry.length == 3);
	eskilib_assert(eskilib_string_equals(entry.value, "ls\0", entry.length));

	result = ncsh_history_add("ls > t.txt\0", 11, &history);
	eskilib_assert(result == N_SUCCESS);

	entry = ncsh_history_get(0, &history);
	eskilib_assert(entry.length == 11);
	eskilib_assert(eskilib_string_equals(entry.value, "ls > t.txt\0", entry.length));

	entry = ncsh_history_get(1, &history);
	eskilib_assert(entry.length == 11);
	eskilib_assert(eskilib_string_equals(entry.value, "echo hello\0", entry.length));

	ncsh_history_free(&history);
}

void ncsh_history_tests(void) {
	eskilib_test_run("ncsh_history_load_file_not_exists_test", ncsh_history_load_file_not_exists_test);
	eskilib_test_run("ncsh_history_load_file_exists_test", ncsh_history_load_file_exists_test);
	eskilib_test_run("ncsh_history_get_empty_file_test", ncsh_history_get_empty_file_test);
	eskilib_test_run("ncsh_history_save_adds_to_file", ncsh_history_save_adds_to_file);
	eskilib_test_run("ncsh_history_save_adds_multiple_to_file", ncsh_history_save_adds_multiple_to_file);
	eskilib_test_run("ncsh_history_load_and_get_entries_test", ncsh_history_load_and_get_entries_test);
	eskilib_test_run("ncsh_history_load_and_get_entries_then_add_entries_test", ncsh_history_load_and_get_entries_then_add_entries_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_history_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
