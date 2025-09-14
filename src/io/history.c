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
#include "../ttyio/ttyio.h"
#include "hashset.h"
#include "history.h"

void history_file_set([[maybe_unused]] Str config_file, History* restrict history, Arena* restrict arena)
{
#if defined(NCSH_HISTORY_TEST) || defined(NCSH_IN_PLACE)
    history->file = *estrdup(&Str_New_Literal(NCSH_HISTORY_FILE), arena);
    return;
#else
    if (!config_file.value || !config_file.length) {
        history->file = Str_New_Literal(NCSH_HISTORY_FILE);
        return;
    }

    if (config_file.length + sizeof(NCSH_HISTORY_FILE) > NCSH_MAX_INPUT) {
        history->file = Str_Empty;
        return;
    }

    history->file = *estrcat(&config_file, &Str_New_Literal(NCSH_HISTORY_FILE), arena);

    debugf("history->file: %s\n", history->file.value);
#endif /* ifdef NCSH_HISTORY_TEST */
}

[[nodiscard]]
enum eresult history_alloc(History* restrict history, Arena* restrict arena)
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
enum eresult history_load(History* restrict history, Arena* restrict arena)
{
    assert(history); assert(history->file.value); assert(arena);

    FILE* file = fopen(history->file.value, "r");
    if (!file || feof(file) || ferror(file)) {
        /*tty_print("ncsh: would you like to create a history file %s? [Y/n]: ", history->file);
            fflush(stdout);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
                tty_perror(NCSH_ERROR_STDIN);
                return E_FAILURE;
            }

        if (character != 'y' || character != 'Y')
            return E_SUCCESS;*/

        file = fopen(history->file.value, "w");
        if (!file || ferror(file)) {
            if (file)
                fclose(file);
            tty_perror("ncsh: Could not load or create history file");
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
enum eresult history_reload(History* restrict history, Arena* restrict arena)
{
    assert(history); assert(history->file.value); assert(arena);

    history->count = 0;

    FILE* file = fopen(history->file.value, "r");
    if (!file || ferror(file) || feof(file)) {
        file = fopen(history->file.value, "w");
        if (!file || ferror(file) || feof(file)) {
            tty_perror("ncsh: Could not load or create history file");
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
enum eresult history_init(Str config_location, History* restrict history, Arena* restrict arena)
{
    assert(history && arena);

    /*enum eresult result;
    if ((result = history_alloc(history, arena)) != E_SUCCESS) {
        tty_perror("ncsh: Error when allocating memory for history");
        return result;
    }*/

    history_file_set(config_location, history, arena);
    if (!history->file.value) {
        tty_fprint(stderr, "ncsh: Could not load history file path.");
        return E_FAILURE;
    }

    /*if ((result = history_load(history, arena)) != E_SUCCESS) {
        tty_perror("ncsh: Error when loading data from history file");
        return result;
    }*/

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_clean__(History* restrict history, Arena* restrict scratch)
{
    assert(history && scratch);
    if (!history->count || !history->entries[0].value) {
        return E_FAILURE_NULL_REFERENCE;
    }

    tty_print("ncsh history: starting to clean history with %zu entries.\n", history->count);

    Hashset hset = {0};
    hashset_malloc(0, scratch, &hset);

    FILE* file = fopen(history->file.value, "w");
    if (!file) {
        tty_perror("ncsh: Could not open .ncsh_history file to clean history");
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = 0; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!hashset_exists(history->entries[i].value, &hset)) {
            hashset_set(history->entries[i], scratch, &hset);

            if (!fputs(history->entries[i].value, file)) {
                tty_perror("ncsh history: Error writing to file");
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
            if (!fputc('\n', file)) {
                tty_perror("ncsh history: Error writing to file");
                fclose(file);
                return E_FAILURE_FILE_OP;
            }
        }
    }

    fclose(file);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult history_clean(History* restrict history, Arena* restrict arena, Arena scratch)
{
    assert(arena);

    enum eresult result;
    result = history_clean__(history, &scratch);
    if (result != E_SUCCESS) {
        tty_fputs("ncsh history: Error when cleaning history data.", stderr);
        return result;
    }

    result = history_reload(history, arena);
    if (result != E_SUCCESS) {
        tty_perror("ncsh history: Error when reloading data from history file");
        return result;
    }

    tty_print("ncsh history: finished cleaning history, history now has %zu entries.\n", history->count);

    return E_SUCCESS;
}

enum eresult history_save(History* restrict history, Arena* restrict scratch)
{
    if (!history || !history->count || !history->entries || !history->entries[0].value) {
        return E_FAILURE_NULL_REFERENCE;
    }

    if (history->count > NCSH_MAX_HISTORY_FILE) {
        return history_clean__(history, scratch);
    }

    size_t pos = history->count > NCSH_MAX_HISTORY_FILE ? history->count - NCSH_MAX_HISTORY_FILE : 0;
    assert(pos == 0 || history->count < pos);
    debugf("history->count %d\n", history->count);
    debugf("history pos %d\n", pos);

    // TODO: history file is full.. ask user if they would like to remove duplicates before saving to condense size of history
    // file removing duplicates saves entries for future autocompletions, but decreases size of overall history file
    // when lots of duplicates exists

    FILE* file = fopen(history->file.value, "w"); // write over entire file each time for now
    if (!file) {
        tty_perror("ncsh: Could not open .ncsh_history file to save history");
        return E_FAILURE_FILE_OP;
    }

    for (size_t i = pos; i < history->count; ++i) {
        if (!history->entries[i].length || !history->entries[i].value) {
            continue;
        }

        if (!fputs(history->entries[i].value, file)) {
            tty_perror("ncsh: Error writing to file");
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
        if (!fputc('\n', file)) {
            tty_perror("ncsh: Error writing to file");
            fclose(file);
            return E_FAILURE_FILE_OP;
        }
    }

    fclose(file);
    return E_SUCCESS;
}

enum eresult history_add(char* restrict line, size_t length, History* restrict history, Arena* restrict arena)
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
Str history_get(size_t position, History* restrict history)
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
int history_command_display(History* restrict history)
{
    assert(history);
    if (!history || !history->count) {
        return EXIT_SUCCESS;
    }

    for (size_t i = 0; i < history->count; ++i) {
        tty_print("%zu %s\n", i + 1, history->entries[i].value);
    }
    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_count(History* restrict history)
{
    assert(history);
    tty_print("history count: %zu\n", history->count);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_clean(History* restrict history, Arena* restrict arena, Arena* restrict scratch)
{
    if (history_clean(history, arena, *scratch) != E_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int history_command_add(Str val, History* restrict history, Arena* restrict arena)
{
    return history_add(val.value, val.length, history, arena) == E_SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE_CONTINUE;
}

void history_remove_entries_shift(size_t offset, History* restrict history)
{
    if (offset + 1 == history->count || !history->count) {
        return;
    }

    memmove(history->entries, history->entries + 1, history->count - 1);
}

[[nodiscard]]
int history_command_remove(Str val, History* restrict history, Arena* restrict arena,
                           Arena* restrict scratch)
{
    assert(val.value); assert(val.length > 0); assert(history);

    if (history_clean(history, arena, *scratch) != E_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    // TODO: is this even needed? didn't we reload history in history clean?
    for (size_t i = 0; i < history->count; ++i) {
        if (estrcmp(history->entries[i], val)) {
            history->entries[i].value = NULL;
            history->entries[i].length = 0;
            history_remove_entries_shift(i, history);
            --history->count;
            tty_print("ncsh history: removed entry: %s\n", val.value);
            return EXIT_SUCCESS;
        }
    }

    return EXIT_FAILURE_CONTINUE;
}
