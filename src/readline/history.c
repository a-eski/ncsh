/* Copyright ncsh (C) by Alex Eski 2024 */
/* history.h: shell command history implementation with clean, count, and display options. */

#include <assert.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../defines.h"
#include "../debug.h"
#include "../eskilib/efile.h"
#include "../eskilib/eresult.h"
#include "../eskilib/str.h"
#include "../ttyterm/ttyterm.h"
#include "hashset.h"
#include "history.h"

void history_file_set(Str config_file, History* rst history, Arena* rst arena)
{
    constexpr size_t history_file_len = sizeof(NCSH_HISTORY_FILE);
#if defined(NCSH_HISTORY_TEST) || defined(NCSH_IN_PLACE)
    history->file = arena_malloc(arena, history_file_len, char);
    memcpy(history->file, NCSH_HISTORY_FILE, history_file_len);
    return;
#endif /* ifdef NCSH_HISTORY_TEST */

    if (!config_file.value || !config_file.length) {
        history->file = NCSH_HISTORY_FILE;
        return;
    }

    if (config_file.length + history_file_len > NCSH_MAX_INPUT) {
        history->file = NULL;
        return;
    }

    history->file = arena_malloc(arena, config_file.length + history_file_len, char);
    memcpy(history->file, config_file.value, config_file.length);
    memcpy(history->file + config_file.length - 1, NCSH_HISTORY_FILE, history_file_len);

    debugf("history->file: %s\n", history->file);
}

[[nodiscard]]
enum eresult history_alloc(History* rst history, Arena* rst arena)
{
    assert(history);
    if (!history) {
        return E_FAILURE_NULL_REFERENCE;
    }

    history->count = 0;
    history->entries = arena_malloc(arena, NCSH_MAX_HISTORY_IN_MEMORY, Str);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_load(History* rst history, Arena* rst arena)
{
    assert(history && history->file && arena);

    FILE* file = fopen(history->file, "r");
    if (!file || feof(file) || ferror(file)) {
        /*term_print("ncsh: would you like to create a history file %s? [Y/n]: ", history->file);
            fflush(stdout);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
                term_perror(NCSH_ERROR_STDIN);
                return E_FAILURE;
            }

        if (character != 'y' || character != 'Y')
            return E_SUCCESS;*/

        file = fopen(history->file, "w");
        if (!file || ferror(file)) {
            if (file)
                fclose(file);
            term_perror("ncsh: Could not load or create history file");
            return E_FAILURE_FILE_OP;
        }
        fclose(file);
        return E_SUCCESS;
    }

    char buffer[NCSH_MAX_INPUT];
    int buffer_length = 0;

    for (size_t i = 0; (buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
         ++i) {
        if (buffer_length <= 0 || !*buffer)
            continue;

        ++history->count;
        history->entries[i].length = (size_t)buffer_length;
        history->entries[i].value = arena_malloc(arena, (uintptr_t)buffer_length, char);
        memcpy(history->entries[i].value, buffer, (size_t)buffer_length);
        history->entries[i].value[buffer_length - 1] = '\0';
    }
    fclose(file);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_reload(History* rst history, Arena* rst arena)
{
    assert(history && history->file && arena);

    history->count = 0;

    FILE* file = fopen(history->file, "r");
    if (!file || ferror(file) || feof(file)) {
        file = fopen(history->file, "w");
        if (!file || ferror(file) || feof(file)) {
            term_perror("ncsh: Could not load or create history file");
            return E_FAILURE_FILE_OP;
        }
        fclose(file);
        return E_SUCCESS;
    }

    char buffer[NCSH_MAX_INPUT];
    int buffer_length = 0;

    for (size_t i = 0; (buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
         ++i) {
        if (buffer_length <= 0 || !*buffer) {
            continue;
        }
        if (!history->entries[i].value)
            continue;

        ++history->count;
        if ((size_t)buffer_length > history->entries[i].length) {
            history->entries[i].value = arena_realloc(arena, (uintptr_t)buffer_length, char, history->entries[i].value,
                                                      history->entries[i].length);
        }
        memcpy(history->entries[i].value, buffer, (size_t)buffer_length);
        history->entries[i].value[buffer_length - 1] = '\0';
        history->entries[i].length = (size_t)buffer_length;
    }
    fclose(file);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_init(Str config_location, History* rst history, Arena* rst arena)
{
    assert(history && arena);

    enum eresult result;
    if ((result = history_alloc(history, arena)) != E_SUCCESS) {
        term_perror("ncsh: Error when allocating memory for history");
        return result;
    }

    history_file_set(config_location, history, arena);
    if (!history->file) {
        return E_FAILURE;
    }

    if ((result = history_load(history, arena)) != E_SUCCESS) {
        term_perror("ncsh: Error when loading data from history file");
        return result;
    }

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_clean(History* rst history, Arena* rst arena, Arena scratch_arena)
{
    assert(history && arena && scratch_arena.start);
    if (!history->count || !history->entries[0].value) {
        return E_FAILURE_NULL_REFERENCE;
    }

    term_print("ncsh history: starting to clean history with %zu entries.\n", history->count);

    Hashset hset = {0};
    hashset_malloc(0, &scratch_arena, &hset);

    FILE* file = fopen(history->file, "w");
    if (!file) {
        term_perror("ncsh: Could not open .ncsh_history file to clean history");
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = 0; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!hashset_exists(history->entries[i].value, &hset)) {
            hashset_set(history->entries[i], &scratch_arena, &hset);

            if (!fputs(history->entries[i].value, file)) {
                term_perror("ncsh history: Error writing to file");
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
            if (!fputc('\n', file)) {
                term_perror("ncsh history: Error writing to file");
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
        }
    }

    fclose(file);

    enum eresult result;
    if ((result = history_reload(history, arena)) != E_SUCCESS) {
        term_perror("ncsh history: Error when reloading data from history file");
        return result;
    }

    term_print("ncsh history: finished cleaning history, history now has %zu entries.\n", history->count);

    return E_SUCCESS;
}

enum eresult history_save(History* rst history)
{
    if (!history || !history->count || !history->entries || !history->entries[0].value) {
        return E_FAILURE_NULL_REFERENCE;
    }

    size_t pos = history->count > NCSH_MAX_HISTORY_FILE ? history->count - NCSH_MAX_HISTORY_FILE : 0;
    assert(pos == 0 || history->count < pos);
    debugf("history->count %d\n", history->count);
    debugf("history pos %d\n", pos);

    // history file is full.. ask user if they would like to remove duplicates before saving to condense size of history
    // file removing duplicates saves entries for future autocompletions, but decreases size of overall history file
    // when lots of duplicates exists

    FILE* file = fopen(history->file, "w"); // write over entire file each time for now
    if (!file) {
        term_perror("ncsh: Could not open .ncsh_history file to save history");
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = pos; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!fputs(history->entries[i].value, file)) {
            term_perror("ncsh: Error writing to file");
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
        if (!fputc('\n', file)) {
            term_perror("ncsh: Error writing to file");
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
    }

    fclose(file);
    return E_SUCCESS;
}

enum eresult history_add(char* rst line, size_t length, History* rst history, Arena* rst arena)
{
    assert(history);
    assert(line);

    if (!history || !line) {
        return E_FAILURE_NULL_REFERENCE;
    }
    else if (!length || !line[0]) {
        return E_FAILURE_ZERO_LENGTH;
    }
    else if (line[length - 1]) {
        return E_FAILURE_BAD_STRING;
    }
    else if (history->count + 1 < history->count) {
        return E_FAILURE_OVERFLOW_PROTECTION;
    }
    else if (history->count + 1 >= NCSH_MAX_HISTORY_IN_MEMORY) {
        return E_NO_OP_MAX_LIMIT_REACHED;
    }
    else if (length == 2 && (line[0] == ' ' || line[0] == '\n')) {
        return E_FAILURE_BAD_STRING;
    }

    assert(length > 0);
    assert(!line[length - 1]);

    history->entries[history->count].length = length;
    history->entries[history->count].value = arena_malloc(arena, length, char);
    memcpy(history->entries[history->count].value, line, length);
    history->entries[history->count].value[length - 1] = '\0';
    ++history->count;
    return E_SUCCESS;
}

[[nodiscard]]
Str history_get(size_t position, History* rst history)
{
    assert(history);

    if (!history || !history->count || !history->entries) {
        return Str_Empty;
    }
    else if (position >= history->count) {
        return Str_Empty;
    }
    else if (history->count - position - 1 > history->count) {
        return Str_Empty;
    }
    else if (position > NCSH_MAX_HISTORY_IN_MEMORY) {
        return history->entries[NCSH_MAX_HISTORY_IN_MEMORY];
    }

    return history->entries[history->count - position - 1];
}

[[nodiscard]]
int history_command_display(History* rst history)
{
    assert(history);
    if (!history || !history->count) {
        return EXIT_SUCCESS;
    }

    for (size_t i = 0; i < history->count; ++i) {
        term_print("%zu %s\n", i + 1, history->entries[i].value);
    }
    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_count(History* rst history)
{
    assert(history);
    term_print("history count: %zu\n", history->count);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_clean(History* rst history, Arena* rst arena, Arena* rst scratch_arena)
{
    if (history_clean(history, arena, *scratch_arena) != E_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_add(char* rst value, size_t value_len, History* rst history, Arena* rst arena)
{
    return history_add(value, value_len, history, arena);
}

void history_remove_entries_shift(size_t offset, History* rst history)
{
    if (offset + 1 == history->count) {
        return;
    }

    for (size_t i = offset; i < history->count - 1; ++i) {
        history->entries[i] = history->entries[i + 1];
    }
}

[[nodiscard]]
int history_command_remove(char* rst value, size_t value_len, History* rst history, Arena* rst arena,
                           Arena* rst scratch_arena)
{
    assert(value);
    assert(value_len > 0);
    assert(history);

    if (history_clean(history, arena, *scratch_arena) != E_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    for (size_t i = 0; i < history->count; ++i) {
        if (estrcmp(history->entries[i].value, history->entries[i].length, value, value_len)) {
            history->entries[i].value = NULL;
            history->entries[i].length = 0;
            history_remove_entries_shift(i, history);
            --history->count;
            term_print("ncsh history: removed entry: %s\n", value);
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE_CONTINUE;
}
