// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_history_h
#define ncsh_history_h

#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#if defined(NCSH_TEST_HISTORY)
	#define NCSH_HISTORY_FILE "/.ncsh_history_test"
	#define NCSH_HISTORY_FILE_LENGTH 20
#else
	#define NCSH_HISTORY_FILE "/.ncsh_history"
	#define NCSH_HISTORY_FILE_LENGTH 14
#endif

#define NCSH_MAX_INPUT 528
#define NCSH_MAX_HISTORY_FILE 2000

struct ncsh_History {
	uint_fast32_t history_count;
	uint_fast32_t file_position;
	bool history_loaded;
	struct eskilib_String config_location;
	struct eskilib_String* entries;
};

enum eskilib_Result ncsh_history_malloc(struct ncsh_History* history);

enum eskilib_Result ncsh_history_load(struct ncsh_History* history);

enum eskilib_Result ncsh_history_save(struct ncsh_History* history);

void ncsh_history_clean(struct ncsh_History* history);

void ncsh_history_free(struct ncsh_History* history);

enum eskilib_Result ncsh_history_add(char* line, uint_fast32_t length, struct ncsh_History* history);

// enum eskilib_Result ncsh_history_remove(char* line, uint_fast32_t length, struct ncsh_History* history);

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history);

uint_fast32_t ncsh_history_command(struct ncsh_History* history);

#endif // !ncsh_history_h

