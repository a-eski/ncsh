// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_history_h
#define ncsh_history_h

#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "ncsh_types.h"
#include "eskilib/eskilib_string.h"

#ifndef NCSH_TEST_HISTORY
#define NCSH_HISTORY_FILE ".ncsh_history"
#else
#define NCSH_HISTORY_FILE ".ncsh_history_test"
#endif /* ifndef NCSH_TEST_HISTORY */

#define NCSH_MAX_HISTORY_FILE 500

struct ncsh_History {
	uint_fast32_t history_count;
	uint_fast32_t file_position;
	bool history_loaded;
	// char history_file_directory[PATH_MAX];
	struct eskilib_String* entries;
};

enum ncsh_Result ncsh_history_malloc(struct ncsh_History* history);

enum ncsh_Result ncsh_history_load(struct ncsh_History* history);

enum ncsh_Result ncsh_history_save(struct ncsh_History* history);

void ncsh_history_clean(struct ncsh_History* history);

void ncsh_history_free(struct ncsh_History* history);

enum ncsh_Result ncsh_history_add(char* line, uint_fast32_t length, struct ncsh_History* history);

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history);

uint_fast32_t ncsh_history_command(struct ncsh_History* history);

#endif // !ncsh_history_h

