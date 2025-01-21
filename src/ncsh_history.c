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
#include "eskilib/eskilib_hashtable.h"
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

eskilib_nodiscard enum eskilib_Result ncsh_history_load(struct ncsh_History *history)
{
    assert(history);
    assert(history->file);

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

eskilib_nodiscard enum eskilib_Result ncsh_history_reload(struct ncsh_History *history)
{
    assert(history);
    assert(history->file);
    int history_original_count = history->count;
    history->count = 0;

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
            history->entries[i].value = realloc(history->entries[i].value, (size_t)buffer_length);
            if (history->entries[i].value == NULL)
                return E_FAILURE_MALLOC;

            eskilib_string_copy(history->entries[i].value, buffer, (size_t)buffer_length);
        }
    }

    fclose(file);

    if (history->count < history_original_count)
    {
        for (int i = history->count; i < history_original_count; ++i)
        {
            if (history->entries[i].value)
                free(history->entries[i].value);
        }
    }

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

    ncsh_history_file_set(config_location, history);
    if (history->file == NULL)
        return E_FAILURE;

    if ((result = ncsh_history_load(history)) != E_SUCCESS)
    {
        perror(RED "ncsh: Error when loading data from history file" RESET);
        fflush(stderr);
        return result;
    }

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_history_clean(struct ncsh_History *history)
{
    assert(history);
    if (history->count == 0 || !history->entries[0].value)
        return E_FAILURE_NULL_REFERENCE;

    printf("ncsh history: starting to clean history with %d entries.\n", history->count);

    struct eskilib_HashTable ht = {0};

    bool ht_malloc_result = eskilib_hashtable_malloc(&ht);
    if (!ht_malloc_result)
        return E_FAILURE_MALLOC;

    FILE *file = fopen(history->file, "w");
    if (file == NULL)
    {
        perror(RED "ncsh: Could not open .ncsh_history file to clean history" RESET);
        return E_FAILURE_FILE_OP;
    }

    for (int i = 0; i < history->count; ++i)
    {
        if (history->entries[i].length == 0 || history->entries[i].value == NULL)
            continue;

        if (!eskilib_hashtable_exists(history->entries[i].value, &ht))
        {
            eskilib_hashtable_set(history->entries[i].value, history->entries[i], &ht);

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
    }

    fclose(file);
    eskilib_hashtable_free(&ht);

    enum eskilib_Result result;
    if ((result = ncsh_history_reload(history)) != E_SUCCESS)
    {
        perror(RED "ncsh history: Error when reloading data from history file" RESET);
        fflush(stderr);
        return result;
    }


    printf("ncsh history: finished cleaning history, history now has %d entries.\n", history->count);

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

eskilib_nodiscard int_fast32_t ncsh_history_command_display(struct ncsh_History *history)
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

eskilib_nodiscard int_fast32_t ncsh_history_command_count(struct ncsh_History *history)
{
    printf("history count: %d\n", history->count);
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

eskilib_nodiscard int_fast32_t ncsh_history_command_clean(struct ncsh_History *history)
{
    if (ncsh_history_clean(history) != E_SUCCESS)
        return NCSH_COMMAND_FAILED_CONTINUE;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

#define HISTORY_COUNT_COMMAND "count"
#define HISTORY_CLEAN_COMMAND "clean"
eskilib_nodiscard int_fast32_t ncsh_history_command(struct ncsh_Args *args, struct ncsh_History *history)
{
    if (args->values[1])
    {
        if (eskilib_string_equals(args->values[1], HISTORY_COUNT_COMMAND, sizeof(HISTORY_COUNT_COMMAND)))
            return ncsh_history_command_count(history);
        else if (eskilib_string_equals(args->values[1], HISTORY_CLEAN_COMMAND, sizeof(HISTORY_CLEAN_COMMAND)))
            return ncsh_history_command_clean(history);
    }

    return ncsh_history_command_display(history);
}
