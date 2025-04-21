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
enum eresult history_init(const struct estr config_location, struct History* const restrict history,
                          struct Arena* const arena);

enum eresult history_save(struct History* const restrict history);

enum eresult history_add(const char* const line, const size_t length, struct History* const restrict history,
                         struct Arena* const arena);

struct estr history_get(const size_t position, struct History* const restrict history);

/* history_command_...
 * History Commands called from builtins when user enters commands like 'history',
 * 'history count', 'history clean', 'history add {directory}', 'history remove {directory}'. */
int history_command_display(const struct History* const restrict history);

int history_command_count(const struct History* const restrict history);

int history_command_clean(struct History* const restrict history, struct Arena* const arena,
                                   struct Arena* const scratch_arena);

int history_command_add(const char* const value, const size_t value_len,
                                 struct History* const restrict history, struct Arena* const arena);

int history_command_remove(const char* const value, const size_t value_len,
                                    struct History* const restrict history, struct Arena* const arena,
                                    struct Arena* const scratch_arena);
