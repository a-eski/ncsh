/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "../eskilib/efile.h"
#include "../eskilib/emap.h"
#include "../eskilib/eresult.h"
#include "../eskilib/estr.h"
#include "history.h"

void history_file_set(const struct estr config_file, struct History* const restrict history, struct Arena* const arena)
{

    constexpr size_t history_file_len = sizeof(NCSH_HISTORY_FILE);
#ifdef NCSH_HISTORY_TEST
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
enum eresult history_alloc(struct History* const restrict history, struct Arena* const arena)
{
    assert(history);
    if (!history) {
        return E_FAILURE_NULL_REFERENCE;
    }

    history->count = 0;
    history->entries = arena_malloc(arena, NCSH_MAX_HISTORY_IN_MEMORY, struct estr);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_load(struct History* const restrict history, struct Arena* const arena)
{
    assert(history && history->file && arena);

    FILE* file = fopen(history->file, "r");
    if (!file || feof(file) || ferror(file)) {
        /*printf("ncsh: would you like to create a history file %s? [Y/n]: ", history->file);
            fflush(stdout);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
                perror(RED NCSH_ERROR_STDIN RESET);
                return E_FAILURE;
            }

        if (character != 'y' || character != 'Y')
            return E_SUCCESS;*/

        file = fopen(history->file, "w");
        if (!file) {
            perror(RED "ncsh: Could not load or create history file" RESET);
            return E_FAILURE_FILE_OP;
        }
        return E_SUCCESS;
    }

    char buffer[NCSH_MAX_INPUT];
    int buffer_length = 0;

    for (size_t i = 0; (buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
         ++i) {
        ++history->count;
        history->entries[i].length = (size_t)buffer_length;
        history->entries[i].value = arena_malloc(arena, (uintptr_t)buffer_length, char);
        memcpy(history->entries[i].value, buffer, (size_t)buffer_length);
    }
    fclose(file);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_reload(struct History* const restrict history, struct Arena* const arena)
{
    assert(history && history->file && arena);

    history->count = 0;

    FILE* file = fopen(history->file, "r");
    if (!file) {
        file = fopen(history->file, "w");
        if (!file) {
            perror(RED "ncsh: Could not load or create history file" RESET);
            return E_FAILURE_FILE_OP;
        }
        return E_SUCCESS;
    }

    char buffer[NCSH_MAX_INPUT];
    int buffer_length = 0;

    for (size_t i = 0; (buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
         ++i) {
        if (buffer_length > 0) {
            ++history->count;
            if ((size_t)buffer_length > history->entries[i].length) {
                history->entries[i].value = arena_realloc(arena, (uintptr_t)buffer_length, char,
                                                          history->entries[i].value, history->entries[i].length);
            }
            memcpy(history->entries[i].value, buffer, (size_t)buffer_length);
            history->entries[i].length = (size_t)buffer_length;
        }
    }
    fclose(file);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_init(const struct estr config_location, struct History* const restrict history,
                          struct Arena* const arena)
{
    assert(history && arena);

    enum eresult result;
    if ((result = history_alloc(history, arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for history" RESET);
        fflush(stderr);
        return result;
    }

    history_file_set(config_location, history, arena);
    if (!history->file) {
        return E_FAILURE;
    }

    if ((result = history_load(history, arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when loading data from history file" RESET);
        fflush(stderr);
        return result;
    }

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_clean(struct History* const restrict history, struct Arena* const arena,
                           struct Arena scratch_arena)
{
    assert(history && arena && scratch_arena.start);
    if (!history->count || !history->entries[0].value) {
        return E_FAILURE_NULL_REFERENCE;
    }

    printf("ncsh history: starting to clean history with %zu entries.\n", history->count);

    struct emap hmap = {0};
    emap_malloc(&scratch_arena, &hmap);

    FILE* file = fopen(history->file, "w");
    if (!file) {
        perror(RED "ncsh: Could not open .ncsh_history file to clean history" RESET);
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = 0; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!emap_exists(history->entries[i].value, &hmap)) {
            emap_set(history->entries[i], &scratch_arena, &hmap);

            if (!fputs(history->entries[i].value, file)) {
                perror(RED "ncsh history: Error writing to file" RESET);
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
            if (!fputc('\n', file)) {
                perror(RED "ncsh history: Error writing to file" RESET);
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
        }
    }

    fclose(file);

    enum eresult result;
    if ((result = history_reload(history, arena)) != E_SUCCESS) {
        perror(RED "ncsh history: Error when reloading data from history file" RESET);
        fflush(stderr);
        return result;
    }

    printf("ncsh history: finished cleaning history, history now has %zu entries.\n", history->count);

    return E_SUCCESS;
}

enum eresult history_save(struct History* const restrict history)
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
        perror(RED "ncsh: Could not open .ncsh_history file to save history" RESET);
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = pos; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!fputs(history->entries[i].value, file)) {
            perror(RED "ncsh: Error writing to file" RESET);
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
        if (!fputc('\n', file)) {
            perror(RED "ncsh: Error writing to file" RESET);
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
    }

    fclose(file);
    return E_SUCCESS;
}

enum eresult history_add(const char* const line, const size_t length, struct History* const restrict history,
                         struct Arena* const arena)
{
    assert(history);
    assert(line);
    assert(length > 0);
    assert(!line[length - 1]);

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

    history->entries[history->count].length = length;
    history->entries[history->count].value = arena_malloc(arena, length, char);
    memcpy(history->entries[history->count].value, line, length);
    history->entries[history->count].value[length - 1] = '\0';
    ++history->count;
    return E_SUCCESS;
}

[[nodiscard]]
struct estr history_get(const size_t position, struct History* const restrict history)
{
    assert(history);

    if (!history || !history->count || !history->entries) {
        return estr_Empty;
    }
    else if (position >= history->count) {
        return estr_Empty;
    }
    else if (history->count - position - 1 > history->count) {
        return estr_Empty;
    }
    else if (position > NCSH_MAX_HISTORY_IN_MEMORY) {
        return history->entries[NCSH_MAX_HISTORY_IN_MEMORY];
    }

    return history->entries[history->count - position - 1];
}

[[nodiscard]]
int_fast32_t history_command_display(const struct History* const restrict history)
{
    assert(history);
    if (!history || !history->count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    for (size_t i = 0; i < history->count; ++i) {
        printf("%zu %s\n", i + 1, history->entries[i].value);
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t history_command_count(const struct History* history)
{
    assert(history);
    printf("history count: %zu\n", history->count);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t history_command_clean(struct History* const restrict history, struct Arena* const arena,
                                   struct Arena* const scratch_arena)
{
    if (history_clean(history, arena, *scratch_arena) != E_SUCCESS) {
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t history_command_add(const char* const value, const size_t value_len,
                                 struct History* const restrict history, struct Arena* const arena)
{
    return history_add(value, value_len, history, arena);
}

void history_remove_entries_shift(const size_t offset, struct History* const restrict history)
{
    if (offset + 1 == history->count) {
        return;
    }

    for (size_t i = offset; i < history->count - 1; ++i) {
        history->entries[i] = history->entries[i + 1];
    }
}

[[nodiscard]]
int_fast32_t history_command_remove(const char* const value, const size_t value_len,
                                    struct History* const restrict history, struct Arena* const arena,
                                    struct Arena* const scratch_arena)
{
    assert(value);
    assert(value_len > 0);
    assert(history);

    if (history_clean(history, arena, *scratch_arena) != E_SUCCESS) {
        return NCSH_COMMAND_FAILED_CONTINUE;
    }

    for (size_t i = 0; i < history->count; ++i) {
        if (estrcmp_c(history->entries[i].value, history->entries[i].length, value, value_len)) {
            history->entries[i].value = NULL;
            history->entries[i].length = 0;
            history_remove_entries_shift(i, history);
            --history->count;
            printf("ncsh history: removed entry: %s\n", value);
            return NCSH_COMMAND_SUCCESS_CONTINUE;
        }
    }

    return NCSH_COMMAND_FAILED_CONTINUE;
}
