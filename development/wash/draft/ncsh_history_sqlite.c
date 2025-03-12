#include <assert.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../eskilib/eskilib_result.h"
#include "../eskilib/eskilib_string.h"

#ifdef NCSH_HISTORY_TEST
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#define NCSH_HISTORY_FILE_LENGTH 19
#else
#define NCSH_HISTORY_FILE "/.ncsh_history"
#define NCSH_HISTORY_FILE_LENGTH 15
#endif

#define NCSH_MAX_HISTORY_FILE 2000

struct ncsh_History {
    uint_fast32_t history_count;
    uint_fast32_t file_position;
    bool history_loaded;
    struct sqlite3* history_db;
    struct eskilib_String* entries;
};

enum eskilib_Result ncsh_history_malloc(struct ncsh_History* history)
{
    assert(history != NULL);
    if (history == NULL)
        return E_FAILURE_NULL_REFERENCE;

    history->history_count = 0;
    history->file_position = 0;
    history->history_loaded = false;
    history->entries = malloc(sizeof(struct eskilib_String) * NCSH_MAX_HISTORY_FILE);
    if (history->entries == NULL)
        return E_FAILURE_MALLOC;

    return E_SUCCESS;
}

void ncsh_history_free(struct ncsh_History* history)
{
    assert(history != NULL);

    for (uint_fast32_t i = 0; i < history->history_count; ++i) {
        free(history->entries[i].value);
    }

    free(history->entries);

    if (history->history_db)
        sqlite3_close(history->history_db);
}

void ncsh_history_exit(struct ncsh_History* history)
{
    if (history->history_loaded) {
        // if (history->history_count > 0)
        // ncsh_history_save(history);
        ncsh_history_free(history);
    }
}

enum eskilib_Result ncsh_history_start(struct ncsh_History* history)
{
    int result = sqlite3_open("ncsh.db", &history->history_db);
    if (result) {
        sqlite3_close(history->history_db);
        if (write(STDERR_FILENO, "Error returned when trying to open ncsh history database.\n", 59) == -1) {
            return E_FAILURE;
        }
        return E_FAILURE_FILE_OP;
    }
    else {
        printf("Opened sqllite3 database");
    }

    printf("Loaded history database");
    return E_SUCCESS;
}

int main(void)
{
    struct ncsh_History history;
    if (ncsh_history_malloc(&history) != E_SUCCESS) {
        ncsh_history_free(&history);
        return EXIT_FAILURE;
    }
    if (ncsh_history_start(&history) != E_SUCCESS) {
        ncsh_history_free(&history);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
