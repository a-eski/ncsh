/* Copyright ncsh (C) by Alex Eski 2024 */
/* history.h: shell command history implementation with clean, count, and display options. */

#pragma once

#include "../arena.h"
#include "../eskilib/eresult.h"
#include "../eskilib/str.h"

#ifdef NCSH_HISTORY_TEST
#define NCSH_HISTORY_FILE "ncsh_history_test"
#else
#define NCSH_HISTORY_FILE "/ncsh_history"
#endif

#define NCSH_MAX_HISTORY_FILE 2000
#define NCSH_MAX_HISTORY_IN_MEMORY 2400

typedef struct {
    size_t count;
    Str file;
    Str* entries;
} History;

/* History Setup and Manipulation */
enum eresult history_init(Str config_location, History* restrict history, Arena* restrict arena);

enum eresult history_save(History* restrict history, Arena* restrict scratch);

enum eresult history_add(char* restrict line, size_t length, History* restrict history, Arena* restrict arena);

Str history_get(size_t position, History* restrict history);

/* history_command_...
 * History Commands called from builtins when user enters commands like 'history',
 * 'history count', 'history clean', 'history add {directory}', 'history remove {directory}'. */
int history_command_display(History* restrict history);

int history_command_count(History* restrict history);

int history_command_clean(History* restrict history, Arena* restrict arena, Arena* restrict scratch_arena);

int history_command_add(Str val, History* restrict history, Arena* restrict arena);

int history_command_remove(Str val, History* restrict history, Arena* restrict arena,
                           Arena* restrict scratch_arena);
