// Copyright (c) ncsh by Alex Eski 2024

#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_file.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"

void ncsh_history_file_set(struct eskilib_String config_file, struct ncsh_History *history)
{
#ifdef NCSH_HISTORY_TEST
    history->file = malloc(sizeof(NCSH_HISTORY_FILE));
    memcpy(history->file, NCSH_HISTORY_FILE, sizeof(NCSH_HISTORY_FILE));
    return;
#endif /* ifdef NCSH_HISTORY_TEST */

    if (config_file.value == NULL || config_file.length == 0)
    {
        history->file = NCSH_HISTORY_FILE;
        return;
    }

    if (config_file.length + sizeof(NCSH_HISTORY_FILE) > NCSH_MAX_INPUT)
    {
        history->file = NULL;
        return;
    }

    history->file = malloc(config_file.length + sizeof(NCSH_HISTORY_FILE));
    memcpy(history->file, config_file.value, config_file.length);
    memcpy(history->file + config_file.length - 1, NCSH_HISTORY_FILE, sizeof(NCSH_HISTORY_FILE));

#ifdef NCSH_DEBUG
    printf("history->file: %s\n", history->file);
#endif /* ifdef NCSH_DEBUG */
}

eskilib_nodiscard enum eskilib_Result ncsh_history_malloc(struct ncsh_History *history)
{
    assert(history);
    if (!history)
        return E_FAILURE_NULL_REFERENCE;

    history->count = 0;
    history->entries = malloc(sizeof(struct eskilib_String) * NCSH_MAX_HISTORY_IN_MEMORY);
    if (history->entries == NULL)
        return E_FAILURE_MALLOC;

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_load(struct eskilib_String config_location, struct ncsh_History *history)
{
    assert(history != NULL);
    if (history == NULL)
        return E_FAILURE_NULL_REFERENCE;

    ncsh_history_file_set(config_location, history);
    if (history->file == NULL)
        return E_FAILURE;

    FILE *file = fopen(history->file, "r");
    if (file == NULL)
    {
        file = fopen(history->file, "w");
        if (file == NULL)
        {
            perror(RED "ncsh: Could not load or create history file" RESET);
            return E_FAILURE_FILE_OP;
        }
        return E_SUCCESS;
    }

    char buffer[NCSH_MAX_INPUT];
    int buffer_length = 0;

    for (size_t i = 0;
         (buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE; ++i)
    {
        if (buffer_length > 0)
        {
            ++history->count;
            history->entries[i].length = (size_t)buffer_length;
            history->entries[i].value = malloc((size_t)buffer_length);
            if (history->entries[i].value == NULL)
                return E_FAILURE_MALLOC;

            eskilib_string_copy(history->entries[i].value, buffer, (size_t)buffer_length);
        }
    }

    fclose(file);

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_init(struct eskilib_String config_location, struct ncsh_History *history)
{
    enum eskilib_Result result;
    if ((result = ncsh_history_malloc(history)) != E_SUCCESS)
    {
        perror(RED "ncsh: Error when allocating memory for history" RESET);
        fflush(stderr);
        return result;
    }

    if ((result = ncsh_history_load(config_location, history)) != E_SUCCESS)
    {
        perror(RED "ncsh: Error when loading data from history file" RESET);
        fflush(stderr);
        return result;
    }

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_clean(struct ncsh_History *history)
{
    (void)history;
    // struct eskilib_HashTable ht = {0};
    // malloc hashtable
    // open file
    // for all entries in memory for history, check if exists in hashtable
    // if not exists, add to hashtable, add to file
    // if exists, continue

    return E_SUCCESS;
}

enum eskilib_Result ncsh_history_save(struct ncsh_History *history)
{
    assert(history->count > 0);
    if (history->count == 0 || !history->entries[0].value)
        return E_FAILURE_NULL_REFERENCE;

    int pos = history->count > NCSH_MAX_HISTORY_FILE ? history->count - NCSH_MAX_HISTORY_FILE : 0;
    assert(pos >= 0);
#ifdef NCSH_DEBUG
    printf("history->count %d\n", history->count);
    printf("pos %d\n", pos);
#endif /* ifdef NCSH_DEBUG */

    // history file is full.. ask user if they would like to remove duplicates before saving to condense size of history
    // file removing duplicates saves entries for future autocompletions, but decreases size of overall history file
    // when lots of duplicates exists

    FILE *file = fopen(history->file, "w"); // write over entire file each time for now
    if (file == NULL)
    {
        perror(RED "ncsh: Could not open .ncsh_history file to save history" RESET);
        return E_FAILURE_FILE_OP;
    }

    for (int i = pos; i < history->count; ++i)
    {
        if (history->entries[i].length == 0 || history->entries[i].value == NULL)
            continue;

        if (!fputs(history->entries[i].value, file))
        {
            perror(RED "ncsh: Error writing to file" RESET);
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
        if (!fputc('\n', file))
        {
            perror(RED "ncsh: Error writing to file" RESET);
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
    }

    fclose(file);
    return E_SUCCESS;
}

/*eskilib_nodiscard enum eskilib_Result history_write_entry_to_file(struct eskilib_String* entry, FILE* file)
{
    assert(file);

    size_t bytes_written;

    bytes_written = fwrite(&entry->length, sizeof(size_t), 1, file);
    if (bytes_written == 0 || feof(file))
        return E_FAILURE;
    else if (ferror(file))
        return E_FAILURE_FILE_OP;

    bytes_written = fwrite(entry->value, sizeof(char), entry->length, file);
    if (bytes_written == 0)
        return E_FAILURE;
    else if (ferror(file))
        return E_FAILURE_FILE_OP;

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_save_v2(struct ncsh_History* history)
{
    assert(history);
    if (!history)
        return E_FAILURE_NULL_REFERENCE;
    if (history->count == 0)
        return E_SUCCESS;

    FILE* file = fopen(history->history_file, "wb");
    if (!file || feof(file) || ferror(file)) {
        perror("Error writing to history file");
        if (file)
            fclose(file);
        return E_FAILURE_FILE_OP;
    }

    if (fwrite(&history->count, sizeof(uint32_t), 1, file) == 0 || feof(file) || ferror(file)) {
        perror("Error writing number of entries to history file, could not write to file");
        fclose(file);
        return E_FAILURE_FILE_OP;
    }

    enum eskilib_Result result;
    for (uint_fast32_t i = 0; i < history->count; ++i) {
        if ((result = history_write_entry_to_file((history->entries + i), file)) != E_SUCCESS) {
            fclose(file);
            return result;
        }
    }

    fclose(file);
    return E_SUCCESS;
}*/

void ncsh_history_free(struct ncsh_History *history)
{
    assert(history);

    for (int i = 0; i < history->count; ++i)
    {
        free(history->entries[i].value);
    }

    free(history->file);
    free(history->entries);
}

#define NCSH_HISTORY_SAVE_ERROR "ncsh history: could not save history."
void ncsh_history_exit(struct ncsh_History *history)
{
    if (history)
    {
        if (history->count > 0)
            ncsh_history_save(history);
        ncsh_history_free(history);
    }
}

eskilib_nodiscard enum eskilib_Result ncsh_history_add(char *line, size_t length, struct ncsh_History *history)
{
    assert(history);
    assert(line);
    assert(length > 0);
    assert(line[length - 1] == '\0');

    if (history == NULL || line == NULL)
        return E_FAILURE_NULL_REFERENCE;
    else if (length == 0)
        return E_FAILURE_ZERO_LENGTH;
    else if (history->count + 1 < history->count)
        return E_FAILURE_OVERFLOW_PROTECTION;
    else if (history->count + 1 >= NCSH_MAX_HISTORY_IN_MEMORY)
        return E_NO_OP_MAX_LIMIT_REACHED;

    history->entries[history->count].length = length;
    history->entries[history->count].value = malloc(length);
    eskilib_string_copy(history->entries[history->count].value, line, length);
    ++history->count;
    return E_SUCCESS;
}

/*eskilib_nodiscard enum eskilib_Result ncsh_history_remove(char* line, uint_fast32_t length, struct ncsh_History* history)
{
    assert(history != NULL);
    assert(line != NULL);
    assert(length != 0);

    if (history == NULL || line == NULL)
        return E_FAILURE_NULL_REFERENCE;
    else if (length == 0)
        return E_FAILURE_ZERO_LENGTH;

    bool entry_found = false;
    int_fast32_t max_length;
    for (uint_fast32_t i = 0; i < history->history_count; ++i) {
        max_length = history->entries[i].length > length ? history->entries[i].length : length;
        if (eskilib_string_equals(line, history->entries[i].value, max_length)) {
            entry_found = true;
            history->entries[i].value = NULL;
            history->entries[i].length = 0;
        }
    }

    if (!entry_found)
        return E_NO_OP;

    FILE* file = fopen(history_file, "w");
    if (file == NULL) {
        perror(RED "ncsh: Could not load or create history file" RESET);
        return E_FAILURE_FILE_OP;
    }

    int_fast32_t buffer_length = 0;
    char buffer[NCSH_MAX_INPUT] = {0};
    while ((buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF) {
        max_length = buffer_length > length ? buffer_length : length;
        if (eskilib_string_equals(line, buffer, max_length)) {
            fputs("", file);
        }
    }

    return E_SUCCESS;
}*/

struct eskilib_String ncsh_history_get(int position, struct ncsh_History *history)
{
    assert(history != NULL);
    // assert(position >= 0);

    if (history == NULL || history->count == 0 || history->entries == NULL)
    {
        return eskilib_String_Empty;
    }
    else if (position < 0)
    {
        return eskilib_String_Empty;
    }
    else if (position >= history->count)
    {
        return eskilib_String_Empty;
    }
    else if (position > NCSH_MAX_HISTORY_IN_MEMORY)
    {
        return history->entries[NCSH_MAX_HISTORY_IN_MEMORY];
    }
    else
    {
        return history->entries[history->count - position - 1];
    }
}

eskilib_nodiscard int_fast32_t ncsh_history_command(struct ncsh_History *history)
{
    assert(history);
    if (!history || history->count == 0)
        return NCSH_COMMAND_SUCCESS_CONTINUE;

    for (int i = 0; i < history->count; ++i)
    {
        printf("%d %s\n", i + 1, history->entries[i].value);
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}
