#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/eskilib/eskilib_string.h"
#include "../src/eskilib/eskilib_test.h"
#include "../src/readline/ncsh_history.h"

void ncsh_history_load_file_not_exists_test(void)
{
    remove(NCSH_HISTORY_FILE);

    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    fclose(file);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_load_file_exists_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    fclose(file);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_get_empty_file_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);
    eskilib_assert(history.count == 0);

    struct eskilib_String entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 0);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_get_null_entries_test(void)
{
    struct ncsh_History history_null_entries = {0};
    history_null_entries.entries = NULL;
    struct eskilib_String entry_history_null = ncsh_history_get(0, &history_null_entries);
    eskilib_assert(entry_history_null.length == 0);
    eskilib_assert(entry_history_null.value == NULL);
}

void ncsh_history_get_position_gt_history_count_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct ncsh_History history = {0};
    enum eskilib_Result result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    size_t position = 10;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_get_position_equals_history_count_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    size_t position = history.count;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_get_position_gt_max_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    // mark position history in memory max.
    size_t position = 10000;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_save_adds_to_file(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    size_t len = 3;
    result = ncsh_history_add("ls\0", len, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    ncsh_history_save(&history);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    char buffer[MAX_INPUT];
    if (!fgets(buffer, sizeof(buffer), file)) {
        eskilib_assert(false);
        fclose(file);
    }
    eskilib_assert(memcmp(buffer, "ls\n", len) == 0);
    fclose(file);

    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_save_adds_multiple_to_file(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    size_t existing_command_len = 3;
    char* existing_command = "ls\n";
    size_t ls_sort_len = 10;
    result = ncsh_history_add("ls | sort\0", ls_sort_len, &history, &arena);
    eskilib_assert(result == E_SUCCESS);
    size_t echo_hello_len = 11;
    result = ncsh_history_add("echo hello\0", echo_hello_len, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    ncsh_history_save(&history);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    char buffer[MAX_INPUT];
    for (uint_fast8_t i = 0; fgets(buffer, sizeof(buffer), file) != NULL; i++) {
        if (i == 0) {
            eskilib_assert(memcmp(buffer, existing_command, existing_command_len) == 0);
        }
        else if (i == 1) {
            eskilib_assert(memcmp(buffer, "ls | sort\n", ls_sort_len) == 0);
        }
        if (i == 2) {
            eskilib_assert(memcmp(buffer, "echo hello\n", echo_hello_len) == 0);
        }
        else {
            break;
        }
    }

    fclose(file);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_get_position_last_entry_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    // try to get the last histroy entry.
    size_t position = history.count - 1;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length != 0);
    eskilib_assert(entry.value != NULL);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_load_and_get_entries_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    struct eskilib_String entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    entry = ncsh_history_get(1, &history);
    eskilib_assert(entry.length == 10);
    eskilib_assert(memcmp(entry.value, "ls | sort\0", entry.length) == 0);

    entry = ncsh_history_get(2, &history);
    eskilib_assert(entry.length == 3);
    eskilib_assert(memcmp(entry.value, "ls\0", entry.length) == 0);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_load_and_get_entries_then_add_entries_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    struct eskilib_String entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    entry = ncsh_history_get(1, &history);
    eskilib_assert(entry.length == 10);
    eskilib_assert(memcmp(entry.value, "ls | sort\0", entry.length) == 0);

    entry = ncsh_history_get(2, &history);
    eskilib_assert(entry.length == 3);
    eskilib_assert(memcmp(entry.value, "ls\0", entry.length) == 0);

    result = ncsh_history_add("ls > t.txt\0", 11, &history, &arena);
    eskilib_assert(result == E_SUCCESS);

    entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(memcmp(entry.value, "ls > t.txt\0", entry.length) == 0);

    entry = ncsh_history_get(1, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    ncsh_history_save(&history);
    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_history_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(ncsh_history_load_file_not_exists_test);
    eskilib_test_run(ncsh_history_load_file_exists_test);
    eskilib_test_run(ncsh_history_get_empty_file_test);
    eskilib_test_run(ncsh_history_get_null_entries_test);
    eskilib_test_run(ncsh_history_get_position_gt_history_count_test);
    eskilib_test_run(ncsh_history_get_position_equals_history_count_test);
    eskilib_test_run(ncsh_history_get_position_gt_max_test);
    eskilib_test_run(ncsh_history_save_adds_to_file);
    eskilib_test_run(ncsh_history_save_adds_multiple_to_file);
    eskilib_test_run(ncsh_history_get_position_last_entry_test);
    eskilib_test_run(ncsh_history_load_and_get_entries_test);
    eskilib_test_run(ncsh_history_load_and_get_entries_then_add_entries_test);

    eskilib_test_finish();
}

int main(void)
{
    ncsh_history_tests();

    return EXIT_SUCCESS;
}
