#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/eskilib/etest.h"
#include "../../src/eskilib/str.h"
#include "../../src/readline/history.h"
#include "../lib/arena_test_helper.h"

void history_load_file_not_exists_test()
{
    remove(NCSH_HISTORY_FILE);

    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eassert(file != NULL);
    fclose(file);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_load_file_exists_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eassert(file != NULL);
    fclose(file);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_get_empty_file_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);
    eassert(history.count == 0);

    Str entry = history_get(0, &history);
    eassert(entry.length == 0);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_get_null_entries_test()
{
    History history_null_entries = {0};
    history_null_entries.entries = NULL;
    Str entry_history_null = history_get(0, &history_null_entries);
    eassert(entry_history_null.length == 0);
    eassert(entry_history_null.value == NULL);
}

void history_get_position_gt_history_count_test()
{
    ARENA_TEST_SETUP;

    History history = {0};
    enum eresult result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    size_t position = 10;

    Str entry = history_get(position, &history);
    eassert(entry.length == 0);
    eassert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_get_position_equals_history_count_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    size_t position = history.count;

    Str entry = history_get(position, &history);
    eassert(entry.length == 0);
    eassert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_get_position_gt_max_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    // mark position history in memory max.
    size_t position = 10000;

    Str entry = history_get(position, &history);
    eassert(entry.length == 0);
    eassert(entry.value == NULL);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_save_adds_to_file()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    size_t len = 3;
    result = history_add("ls\0", len, &history, &arena);
    eassert(result == E_SUCCESS);

    history_save(&history);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eassert(file != NULL);
    char buffer[MAX_INPUT];
    if (!fgets(buffer, sizeof(buffer), file)) {
        eassert(false);
        fclose(file);
    }
    eassert(memcmp(buffer, "ls\n", len) == 0);
    fclose(file);

    ARENA_TEST_TEARDOWN;
}

void history_save_adds_multiple_to_file()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    size_t existing_command_len = 3;
    char* existing_command = "ls\n";
    size_t ls_sort_len = 10;
    result = history_add("ls | sort\0", ls_sort_len, &history, &arena);
    eassert(result == E_SUCCESS);
    size_t echo_hello_len = 11;
    result = history_add("echo hello\0", echo_hello_len, &history, &arena);
    eassert(result == E_SUCCESS);

    history_save(&history);

    FILE* file = fopen(NCSH_HISTORY_FILE, "r");
    eassert(file != NULL);
    char buffer[MAX_INPUT];
    for (uint_fast8_t i = 0; fgets(buffer, sizeof(buffer), file) != NULL; i++) {
        if (i == 0) {
            eassert(memcmp(buffer, existing_command, existing_command_len) == 0);
        }
        else if (i == 1) {
            eassert(memcmp(buffer, "ls | sort\n", ls_sort_len) == 0);
        }
        if (i == 2) {
            eassert(memcmp(buffer, "echo hello\n", echo_hello_len) == 0);
        }
        else {
            break;
        }
    }

    fclose(file);
    ARENA_TEST_TEARDOWN;
}

void history_get_position_last_entry_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    // try to get the last histroy entry.
    size_t position = history.count - 1;

    Str entry = history_get(position, &history);
    eassert(entry.length != 0);
    eassert(entry.value != NULL);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_load_and_get_entries_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    Str entry = history_get(0, &history);
    eassert(entry.length == 11);
    eassert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    entry = history_get(1, &history);
    eassert(entry.length == 10);
    eassert(memcmp(entry.value, "ls | sort\0", entry.length) == 0);

    entry = history_get(2, &history);
    eassert(entry.length == 3);
    eassert(memcmp(entry.value, "ls\0", entry.length) == 0);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

void history_load_and_get_entries_then_add_entries_test()
{
    ARENA_TEST_SETUP;

    enum eresult result;
    History history = {0};
    result = history_init(Str_Empty, &history, &arena);
    eassert(result == E_SUCCESS);

    Str entry = history_get(0, &history);
    eassert(entry.length == 11);
    eassert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    entry = history_get(1, &history);
    eassert(entry.length == 10);
    eassert(memcmp(entry.value, "ls | sort\0", entry.length) == 0);

    entry = history_get(2, &history);
    eassert(entry.length == 3);
    eassert(memcmp(entry.value, "ls\0", entry.length) == 0);

    result = history_add("ls > t.txt\0", 11, &history, &arena);
    eassert(result == E_SUCCESS);

    entry = history_get(0, &history);
    eassert(entry.length == 11);
    eassert(memcmp(entry.value, "ls > t.txt\0", entry.length) == 0);

    entry = history_get(1, &history);
    eassert(entry.length == 11);
    eassert(memcmp(entry.value, "echo hello\0", entry.length) == 0);

    history_save(&history);
    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(history_load_file_not_exists_test);
    etest_run(history_load_file_exists_test);
    etest_run(history_get_empty_file_test);
    etest_run(history_get_null_entries_test);
    etest_run(history_get_position_gt_history_count_test);
    etest_run(history_get_position_equals_history_count_test);
    etest_run(history_get_position_gt_max_test);
    etest_run(history_save_adds_to_file);
    etest_run(history_save_adds_multiple_to_file);
    etest_run(history_get_position_last_entry_test);
    etest_run(history_load_and_get_entries_test);
    etest_run(history_load_and_get_entries_then_add_entries_test);

    etest_finish();

    return EXIT_SUCCESS;
}
