// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_history_h
#define ncsh_history_h

#include <sqlite3.h>

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH_MAX_HISTORY_FILE 2000

struct ncsh_History {
	uint_fast32_t history_count;
	uint_fast32_t file_position;
	bool history_loaded;
	sqlite3* history_db;
	struct eskilib_String* entries;
};

enum eskilib_Result ncsh_history_malloc(struct ncsh_History* history);

enum eskilib_Result ncsh_history_load(struct eskilib_String config_location, struct ncsh_History* history);

enum eskilib_Result ncsh_history_save(struct ncsh_History* history);

void ncsh_history_clean(struct ncsh_History* history);

void ncsh_history_free(struct ncsh_History* history);

enum eskilib_Result ncsh_history_add(char* line, uint_fast32_t length, struct ncsh_History* history);

enum eskilib_Result ncsh_history_remove(char* line, uint_fast32_t length, struct ncsh_History* history);

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history);

uint_fast32_t ncsh_history_command(struct ncsh_History* history);

#endif // !ncsh_history_h

	/*int result = sqlite3_open_v2("ncsh_history", &history->history_db, SQLITE_OPEN_READONLY | SQLITE_OPEN_CREATE, NULL);
	if (result != SQLITE_OK) {
		sqlite3_close(history->history_db);
		if (write(STDERR_FILENO, "Error returned when trying to open ncsh history database.", 58) == -1) {
			return E_FAILURE;
		}
		return E_FAILURE_FILE_OP;
	}*/
