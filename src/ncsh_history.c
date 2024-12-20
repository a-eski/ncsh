// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <linux/limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_file.h"
#include "eskilib/eskilib_string.h"

void ncsh_history_file_set(struct eskilib_String config_file, struct ncsh_History* history) {
	#ifdef NCSH_HISTORY_TEST
	history->history_file = malloc(NCSH_HISTORY_FILE_LENGTH);
	memcpy(history->history_file, NCSH_HISTORY_FILE, NCSH_HISTORY_FILE_LENGTH);
	return;
	#endif /* ifdef NCSH_HISTORY_TEST */
	if (config_file.value == NULL || config_file.length == 0) {
		history->history_file = NCSH_HISTORY_FILE;
		return;
	}

	if (config_file.length + NCSH_HISTORY_FILE_LENGTH > NCSH_MAX_INPUT) {
		history->history_file = NULL;
		return;
	}

	history->history_file = malloc(config_file.length + NCSH_HISTORY_FILE_LENGTH);
	memcpy(history->history_file, config_file.value, config_file.length);
	strncat(history->history_file, NCSH_HISTORY_FILE, NCSH_HISTORY_FILE_LENGTH);
}

enum eskilib_Result ncsh_history_malloc(struct ncsh_History* history) {
	assert(history != NULL);
	if (history == NULL)
		return E_FAILURE_NULL_REFERENCE;

	history->history_count = 0;
	history->file_position = 0;
	history->history_loaded = false;
	history->entries = malloc(sizeof(struct eskilib_String) * NCSH_MAX_HISTORY_FILE);
	if (history->entries == NULL)
		return E_FAILURE_MALLOC;

	return E_SUCCESS;
}

enum eskilib_Result ncsh_history_load(struct eskilib_String config_location, struct ncsh_History* history) {
	assert(history != NULL);
	if (history == NULL)
		return E_FAILURE_NULL_REFERENCE;

	ncsh_history_file_set(config_location, history);
	if (history->history_file == NULL)
		return E_FAILURE;

	FILE* file = fopen(history->history_file, "r");
	if (file == NULL) {
		file = fopen(history->history_file, "w");
		if (file == NULL)
		{
			perror(RED "ncsh: Could not load or create history file" RESET);
			return E_FAILURE_FILE_OP;
		}
		return E_SUCCESS;
	}

	char buffer[NCSH_MAX_INPUT];
	int_fast32_t buffer_length = 0;

	for (size_t i = 0;
		(buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
		++i) {
		if (buffer_length > 0) {
			++history->file_position;
			history->entries[i].length = buffer_length;
			history->entries[i].value = malloc(buffer_length);
			if (history->entries[i].value == NULL)
				return E_FAILURE_MALLOC;

			eskilib_string_copy(history->entries[i].value, buffer, buffer_length);
		}
	}

	history->history_count = history->file_position;

	fclose(file);

	history->history_loaded = true;
	return E_SUCCESS;
}

enum eskilib_Result ncsh_history_save(struct ncsh_History* history) {
	assert(history != NULL);
	if (history == NULL || !history->entries[0].value)
		return E_FAILURE_NULL_REFERENCE;

	FILE* file = fopen(history->history_file, "a");
	if (file == NULL) {
		perror(RED "ncsh: Could not open .ncsh_history file to save history" RESET);
		return E_FAILURE_FILE_OP;
	}

	for (uint_fast32_t i = history->file_position == 0 ? 0 : history->file_position - 1;
		i < history->history_count;
		++i) {
		if (history->entries[i].length == 0 || history->entries[i].value == NULL)
			continue;

		if (!fputs(history->entries[i].value, file)) {
			perror(RED "ncsh: Error writing to file" RESET);
			fclose(file);
			return E_FAILURE_FILE_OP;
		}
		if (!fputc('\n', file)) {
			perror(RED "ncsh: Error writing to file" RESET);
			fclose(file);
			return E_FAILURE_FILE_OP;
		}
	}

	fclose(file);
	return E_SUCCESS;
}

void ncsh_history_exit(struct ncsh_History* history) {
	if (history->history_loaded) {
		if (history->history_count > 0)
			ncsh_history_save(history);
		ncsh_history_free(history);
	}
}

void ncsh_history_free(struct ncsh_History* history) {
	assert(history != NULL);

	for (uint_fast32_t i = 0; i < history->history_count; ++i) {
		free(history->entries[i].value);
	}

	free(history->history_file);
	free(history->entries);
}

enum eskilib_Result ncsh_history_add(char* line, size_t length, struct ncsh_History* history) {
	assert(history);
	assert(line);
	assert(length > 0);

	if (history == NULL || line == NULL)
		return E_FAILURE_NULL_REFERENCE;
	else if (length == 0)
		return E_FAILURE_ZERO_LENGTH;
	else if (history->history_count + 1 < history->history_count)
		return E_FAILURE_OVERFLOW_PROTECTION;
	else if (history->history_count >= NCSH_MAX_HISTORY_FILE)
		return E_NO_OP_MAX_LIMIT_REACHED;

	history->entries[history->history_count].length = length;
	history->entries[history->history_count].value = malloc(length);
	eskilib_string_copy(history->entries[history->history_count].value, line, length);
	++history->history_count;
	return E_SUCCESS;
}

/*enum eskilib_Result ncsh_history_remove(char* line, uint_fast32_t length, struct ncsh_History* history) {
	assert(history != NULL);
	assert(line != NULL);
	assert(length != 0);

	if (history == NULL || line == NULL)
		return E_FAILURE_NULL_REFERENCE;
	else if (length == 0)
		return E_FAILURE_ZERO_LENGTH;
	else if (history->history_loaded == false)
		return E_NO_OP;

	bool entry_found = false;
	int_fast32_t max_length;
	for (uint_fast32_t i = 0; i < history->history_count; ++i) {
		max_length = history->entries[i].length > length ? history->entries[i].length : length;
		if (eskilib_string_equals(line, history->entries[i].value, max_length)) {
			entry_found = true;
			history->entries[i].value = NULL;
			history->entries[i].length = 0;
		}
	}

	if (!entry_found)
		return E_NO_OP;

	FILE* file = fopen(history_file, "w");
	if (file == NULL) {
		perror(RED "ncsh: Could not load or create history file" RESET);
		return E_FAILURE_FILE_OP;
	}

	int_fast32_t buffer_length = 0;
	char buffer[NCSH_MAX_INPUT] = {0};
	while ((buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF) {
		max_length = buffer_length > length ? buffer_length : length;
		if (eskilib_string_equals(line, buffer, max_length)) {
			fputs("", file);
		}
	}

	return E_SUCCESS;
}*/

struct eskilib_String ncsh_history_get(uint_fast32_t position, struct ncsh_History* history) {
	assert(history != NULL);

	if (history == NULL) {
		return eskilib_String_Empty;
	}
	else if (history->entries == NULL) {
		return eskilib_String_Empty;
	}
	else if (!history->history_loaded) {
		return eskilib_String_Empty;
	}
	else if (history->history_count == 0 && position == 0) {
		return eskilib_String_Empty;
	}
	else if (position >= history->history_count) {
		return eskilib_String_Empty;
	}
	else if (position > NCSH_MAX_HISTORY_FILE) {
		return history->entries[NCSH_MAX_HISTORY_FILE];
	}
	else {
		return history->entries[history->history_count - position - 1];
	}
}

uint_fast32_t ncsh_history_command(struct ncsh_History* history) {
	assert(history != NULL);
	if (history == NULL || history->history_count == 0)
		return 1;

	for (uint_fast32_t i = 0; i < history->history_count; ++i) {
		printf("%lu %s\n", i + 1, history->entries[i].value);
	}
	return 1;
}

