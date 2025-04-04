/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdint.h>

#include "../eskilib/eskilib_result.h"
#include "../eskilib/eskilib_string.h"
#include "../ncsh_arena.h"
#include "../ncsh_configurables.h"
#include "../ncsh_parser.h"

#ifdef NCSH_HISTORY_TEST
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#else
#define NCSH_HISTORY_FILE "/.ncsh_history"
#endif

#define NCSH_MAX_HISTORY_FILE 2000
#define NCSH_MAX_HISTORY_IN_MEMORY 2400

struct ncsh_History {
    size_t count;
    char* file;
    struct eskilib_String* entries;
};

/* History Setup and Manipulation */
enum eskilib_Result ncsh_history_init(const struct eskilib_String config_location,
                                      struct ncsh_History* const restrict history,
                                      struct ncsh_Arena* const arena);

enum eskilib_Result ncsh_history_save(struct ncsh_History* const restrict history);

enum eskilib_Result ncsh_history_add(const char* const line,
                                     const size_t length,
                                     struct ncsh_History* const restrict history,
                                     struct ncsh_Arena* const arena);

struct eskilib_String ncsh_history_get(const size_t position,
                                       struct ncsh_History* const restrict history);

/* ncsh_history_command_...
 * History Commands called from ncsh_builtins when user enters commands like 'history',
 * 'history count', 'history clean', 'history add {directory}', 'history remove {directory}'. */
int_fast32_t ncsh_history_command_display(const struct ncsh_History* const restrict history);

int_fast32_t ncsh_history_command_count(const struct ncsh_History* const restrict history);

int_fast32_t ncsh_history_command_clean(struct ncsh_History* const restrict history,
                                        struct ncsh_Arena* const arena,
					struct ncsh_Arena* const scratch_arena);

int_fast32_t ncsh_history_command_add(const char* const value,
                                      const size_t value_len,
                                      struct ncsh_History* const restrict history,
                                      struct ncsh_Arena* const arena);

int_fast32_t ncsh_history_command_remove(const char* const value,
					 const size_t value_len,
                                         struct ncsh_History* const restrict history,
                                         struct ncsh_Arena* const arena,
					 struct ncsh_Arena* const scratch_arena);
