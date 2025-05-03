/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include "../arena.h"
#include "../configurables.h"
#include "../eskilib/eresult.h"
#include "../eskilib/estr.h"
#include "../parser.h"

#ifdef NCSH_HISTORY_TEST
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#else
#define NCSH_HISTORY_FILE "/.ncsh_history"
#endif

#define NCSH_MAX_HISTORY_FILE 2000
#define NCSH_MAX_HISTORY_IN_MEMORY 2400

struct History {
    size_t count;
    char* file;
    struct estr* entries;
};

/* History Setup and Manipulation */
enum eresult history_init(struct estr config_location, struct History* restrict history, struct Arena* restrict arena);

enum eresult history_save(struct History* restrict history);

enum eresult history_add(char* restrict line, size_t length, struct History* restrict history,
                         struct Arena* restrict arena);

struct estr history_get(size_t position, struct History* restrict history);

/* history_command_...
 * History Commands called from builtins when user enters commands like 'history',
 * 'history count', 'history clean', 'history add {directory}', 'history remove {directory}'. */
int history_command_display(struct History* restrict history);

int history_command_count(struct History* restrict history);

int history_command_clean(struct History* restrict history, struct Arena* restrict arena,
                          struct Arena* restrict scratch_arena);

int history_command_add(char* restrict value, size_t value_len, struct History* restrict history,
                        struct Arena* restrict arena);

int history_command_remove(char* restrict value, size_t value_len, struct History* restrict history,
                           struct Arena* restrict arena, struct Arena* restrict scratch_arena);
