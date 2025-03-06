// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_HISTORY_H_
#define NCSH_HISTORY_H_

#include <stdint.h>

#include "../eskilib/eskilib_result.h"
#include "../eskilib/eskilib_string.h"
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
    int count;
    char* file;
    struct eskilib_String* entries;
};

/* History Setup and Manipulation */
enum eskilib_Result ncsh_history_init(const struct eskilib_String config_location, struct ncsh_History* const restrict history);

void ncsh_history_exit(struct ncsh_History* const restrict history);

enum eskilib_Result ncsh_history_add(const char* const line, const size_t length, struct ncsh_History* const restrict history);

struct eskilib_String ncsh_history_get(const int position, struct ncsh_History* const restrict history);

/* ncsh_history_command_...
 * History Commands called from ncsh_builtins when user enters commands like 'history',
 * 'history count', 'history clean', 'history add {directory}', 'history remove {directory}'. */
int_fast32_t ncsh_history_command_display(const struct ncsh_History* const restrict history);
int_fast32_t ncsh_history_command_count(const struct ncsh_History* const restrict history);
int_fast32_t ncsh_history_command_clean(struct ncsh_History* const restrict history);
int_fast32_t ncsh_history_command_add(const char* const value, const size_t value_len,struct ncsh_History* const restrict history);
int_fast32_t ncsh_history_command_remove(const char* const value, const size_t value_len, struct ncsh_History* const restrict history);

#endif // !NCSH_HISTORY_H_
