#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_test.h"

#include "../ncsh_history.h"

void ncsh_history_load_file_not_exists_test(void)
{
    remove(NCSH_HISTORY_FILE);

    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    FILE *file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    fclose(file);

    ncsh_history_exit(&history);
}

void ncsh_history_load_file_exists_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    FILE *file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    fclose(file);

    ncsh_history_exit(&history);
}

void ncsh_history_get_empty_file_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);
    eskilib_assert(history.count == 0);

    struct eskilib_String entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 0);

    ncsh_history_exit(&history);
}

void ncsh_history_get_negative_input_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    struct eskilib_String entry = ncsh_history_get(-1, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    ncsh_history_exit(&history);
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
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    int position = 10;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    ncsh_history_exit(&history);
}

void ncsh_history_get_position_equals_history_count_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    // mark history count as greater than position.
    history.count = 2;
    int position = history.count;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    // set history count back to avoid segv
    history.count = 0;

    ncsh_history_exit(&history);
}

void ncsh_history_get_position_gt_max_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    // mark position history in memory max.
    int position = 10000;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length == 0);
    eskilib_assert(entry.value == NULL);

    ncsh_history_exit(&history);
}

void ncsh_history_save_adds_to_file(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    result = ncsh_history_add("ls\0", 3, &history);
    eskilib_assert(result == E_SUCCESS);

    ncsh_history_exit(&history);

    FILE *file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    char buffer[MAX_INPUT];
    if (!fgets(buffer, sizeof(buffer), file))
    {
        eskilib_assert(false);
        fclose(file);
    }
    eskilib_assert(eskilib_string_equals(buffer, "ls\n", MAX_INPUT));
    fclose(file);
}

void ncsh_history_save_adds_multiple_to_file(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    char *existing_command = "ls\n";
    result = ncsh_history_add("ls | sort\0", 10, &history);
    eskilib_assert(result == E_SUCCESS);
    result = ncsh_history_add("echo hello\0", 11, &history);
    eskilib_assert(result == E_SUCCESS);

    ncsh_history_exit(&history);

    FILE *file = fopen(NCSH_HISTORY_FILE, "r");
    eskilib_assert(file != NULL);
    char buffer[MAX_INPUT];
    for (uint_fast8_t i = 0; fgets(buffer, sizeof(buffer), file) != NULL; i++)
    {
        if (i == 0)
        {
            eskilib_assert(eskilib_string_equals(buffer, existing_command, MAX_INPUT));
        }
        else if (i == 1)
        {
            eskilib_assert(eskilib_string_equals(buffer, "ls | sort\n", MAX_INPUT));
        }
        if (i == 2)
        {
            eskilib_assert(eskilib_string_equals(buffer, "echo hello\n", MAX_INPUT));
        }
        else
        {
            break;
        }
    }

    fclose(file);
}

void ncsh_history_get_position_last_entry_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    // try to get the last histroy entry.
    int position = history.count - 1;

    struct eskilib_String entry = ncsh_history_get(position, &history);
    eskilib_assert(entry.length != 0);
    eskilib_assert(entry.value != NULL);

    ncsh_history_exit(&history);
}

void ncsh_history_load_and_get_entries_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

    struct eskilib_String entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(eskilib_string_equals(entry.value, "echo hello\0", entry.length));

    entry = ncsh_history_get(1, &history);
    eskilib_assert(entry.length == 10);
    eskilib_assert(eskilib_string_equals(entry.value, "ls | sort\0", entry.length));

    entry = ncsh_history_get(2, &history);
    eskilib_assert(entry.length == 3);
    eskilib_assert(eskilib_string_equals(entry.value, "ls\0", entry.length));

    ncsh_history_exit(&history);
}

void ncsh_history_load_and_get_entries_then_add_entries_test(void)
{
    enum eskilib_Result result;
    struct ncsh_History history = {0};
    result = ncsh_history_init(eskilib_String_Empty, &history);
    eskilib_assert(result == E_SUCCESS);

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
    eskilib_assert(result == E_SUCCESS);

    entry = ncsh_history_get(0, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(eskilib_string_equals(entry.value, "ls > t.txt\0", entry.length));

    entry = ncsh_history_get(1, &history);
    eskilib_assert(entry.length == 11);
    eskilib_assert(eskilib_string_equals(entry.value, "echo hello\0", entry.length));

    ncsh_history_exit(&history);
}

void ncsh_history_tests(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_history_load_file_not_exists_test", ncsh_history_load_file_not_exists_test);
    eskilib_test_run("ncsh_history_load_file_exists_test", ncsh_history_load_file_exists_test);
    eskilib_test_run("ncsh_history_get_empty_file_test", ncsh_history_get_empty_file_test);
    eskilib_test_run("ncsh_history_get_negative_input_test", ncsh_history_get_negative_input_test);
    eskilib_test_run("ncsh_history_get_null_entries_test", ncsh_history_get_null_entries_test);
    eskilib_test_run("ncsh_history_get_position_gt_history_count_test", ncsh_history_get_position_gt_history_count_test);
    eskilib_test_run("ncsh_history_get_position_equals_history_count_test", ncsh_history_get_position_equals_history_count_test);
    eskilib_test_run("ncsh_history_get_position_gt_max_test", ncsh_history_get_position_gt_max_test);
    eskilib_test_run("ncsh_history_save_adds_to_file", ncsh_history_save_adds_to_file);
    eskilib_test_run("ncsh_history_save_adds_multiple_to_file", ncsh_history_save_adds_multiple_to_file);
    eskilib_test_run("ncsh_history_get_position_last_entry_test", ncsh_history_get_position_last_entry_test);
    eskilib_test_run("ncsh_history_load_and_get_entries_test", ncsh_history_load_and_get_entries_test);
    eskilib_test_run("ncsh_history_load_and_get_entries_then_add_entries_test",
                     ncsh_history_load_and_get_entries_then_add_entries_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_history_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
