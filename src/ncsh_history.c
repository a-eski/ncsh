// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <linux/limits.h>
#include <stdio.h>
#include <assert.h>

#include "ncsh_history.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_file.h"
#include "eskilib/eskilib_string.h"

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

void ncsh_history_file_path(char* path, char* destination) {
	strcat(destination, path);
	strcat(destination, NCSH_HISTORY_FILE);
}

enum eskilib_Result ncsh_history_load(struct ncsh_History* history) {
	assert(history != NULL);

	// char file_buffer[PATH_MAX];
	// ncsh_history_file_path(history->history_file_directory, file_buffer);

	FILE* file = fopen(NCSH_HISTORY_FILE, "r");
	if (file == NULL) {
		file = fopen(NCSH_HISTORY_FILE, "w");
		if (file == NULL)
		{
			perror("Could not load or create history file.");
			return E_FAILURE_FILE_OP;
		}
	}

	char buffer[MAX_INPUT];
	int_fast32_t buffer_length = 0;

	for (uint_fast32_t i = 0;
		(buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF && i < NCSH_MAX_HISTORY_FILE;
		i++) {
		if (buffer_length > 0) {
			++history->file_position;
			history->entries[i].length = buffer_length;
			history->entries[i].value = malloc(sizeof(char) * buffer_length);
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

	// char file_buffer[PATH_MAX];
	// ncsh_history_file_path(history->history_file_directory, file_buffer);

	FILE* file = fopen(NCSH_HISTORY_FILE, "a");
	if (file == NULL) {
		fputs("Could not create or open .ncsh_history file.\n", stderr);
		return E_FAILURE_FILE_OP;
	}

	for (uint_fast32_t i = history->file_position == 0 ? 0 : history->file_position - 1;
		i < history->history_count;
		i++) {
		if (!fputs(history->entries[i].value, file)) {
			perror("Error writing to file");
			fclose(file);
			return E_FAILURE_FILE_OP;
		}
		if (!fputc('\n', file)) {
			perror("Error writing to file");
			fclose(file);
			return E_FAILURE_FILE_OP;
		}
	}

	fclose(file);
	return E_SUCCESS;
}

void ncsh_history_free(struct ncsh_History* history) {
	assert(history != NULL);

	for (uint_fast32_t i = 0; i < history->history_count; i++) {
		free(history->entries[i].value);
	}

	free(history->entries);
}

enum eskilib_Result ncsh_history_add(char* line, uint_fast32_t length, struct ncsh_History* history) {
	assert(history != NULL);
	assert(line != NULL);
	assert(length != 0);

	if (history == NULL || line == NULL)
		return E_FAILURE_NULL_REFERENCE;
	else if (length == 0)
		return E_ZERO_LENGTH;
	else if (history->history_count + 1 < history->history_count)
		return E_OVERFLOW_PROTECTION;
	else if (history->history_count >= NCSH_MAX_HISTORY_FILE)
		return E_MAX_LIMIT_REACHED;

	history->entries[history->history_count].length = length;
	history->entries[history->history_count].value = malloc(sizeof(char) * length);
	eskilib_string_copy(history->entries[history->history_count].value, line, length);
	++history->history_count;
	return E_SUCCESS;
}

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

	for (uint_fast32_t i = 0; i < history->history_count; i++) {
		printf("%lu %s\n", i + 1, history->entries[i].value);
	}
	return 1;
}

