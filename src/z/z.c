// Copyright (c) z by Alex Eski 2024

#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/limits.h>

#include "z.h"
#include "../eskilib/eskilib_colors.h"
#include "../ncsh_defines.h"

double z_score(struct z_Directory* directory, time_t now) {
	assert(directory);

	time_t duration = now - directory->last_accessed;

	if (duration < Z_HOUR)
		return directory->rank * 4.0;
	else if (duration < Z_DAY)
		return directory->rank * 2.0;
	else if (duration < Z_WEEK)
		return directory->rank * 0.5;
	else
		return directory->rank * 0.25;
}

struct z_Directory* z_find_match_add(char* target, size_t target_length, struct z_Database* db) {
	assert(db && target && target_length > 0);
	if (!db || !target || target_length < 2)
		return NULL;

	struct z_Directory* current_match = NULL;
	time_t now = time(NULL);

	for (uint_fast32_t i = 0; i < db->count; ++i) {
		if (eskilib_string_contains_s2((db->dirs + i)->path, (db->dirs + i)->path_length, target, target_length)) {
			if (!current_match || z_score(current_match, now) < z_score((db->dirs + i), now)) {
				current_match = (db->dirs + i);
			}
		}
	}

	if (current_match) {
		++current_match->rank;
		current_match->last_accessed = time(NULL);
		// printf("Best match: %s\n", current_match->path);
		// printf("Best match score: %f\n", current_match->rank);
	}

	return current_match;
}

struct z_Directory* z_find_match(char* target, size_t target_length, const char* cwd, size_t cwd_length, struct z_Database* db) {
	assert(db);
	if (!db || !target || target_length == 0 || db->count == 0 || !cwd || cwd_length == 0)
		return NULL;

	struct z_Directory* current_match = NULL;
	time_t now = time(NULL);

	for (uint_fast32_t i = 0; i < db->count; ++i) {
		size_t compare_length = cwd_length > (db->dirs + i)->path_length ? cwd_length : (db->dirs + i)->path_length;
		if (eskilib_string_contains_s2((db->dirs + i)->path, (db->dirs + i)->path_length, target, target_length) &&
			!eskilib_string_equals((db->dirs + i)->path, (char*)cwd, compare_length)) {
			if (!current_match || z_score(current_match, now) < z_score((db->dirs + i), now)) {
				current_match = (db->dirs + i);
			}
		}
	}

	if (current_match) {
		++current_match->rank;
		current_match->last_accessed = time(NULL);
		// printf("Best match: %s\n", current_match->path);
		// printf("Best match score: %f\n", current_match->rank);
	}

	return current_match;
}

enum z_Result z_write_entry_to_file(struct z_Directory* dir, FILE* file) {
	assert(dir && file);

	size_t bytes_read;

	bytes_read = fwrite(&dir->rank, sizeof(double), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	bytes_read = fwrite(&dir->last_accessed, sizeof(time_t), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	bytes_read = fwrite(&dir->path_length, sizeof(uint32_t), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	bytes_read = fwrite(dir->path, sizeof(char), dir->path_length, file);
	if (bytes_read == 0)
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	return Z_SUCCESS;
}

enum z_Result z_write_to_database_file(struct z_Database* db) {
	assert(db);
	if (!db)
		return Z_NULL_REFERENCE;
	if (db->count == 0)
		return Z_SUCCESS;

	FILE* file = fopen(db->database_file, "wb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error writing to z database file");
		if (file)
			fclose(file);
		return Z_FILE_ERROR;
	}

	if (fwrite(&db->count, sizeof(uint32_t), 1, file) == 0 || feof(file) || ferror(file)) {
		perror("Error writing number of entries to z database file, could not write to database file");
		fclose(file);
		return Z_FILE_ERROR;
	}

	enum z_Result result;
	for (uint_fast32_t i = 0; i < db->count; ++i) {
		if ((result = z_write_entry_to_file((db->dirs + i), file)) != Z_SUCCESS) {
			fclose(file);
			return result;
		}
	}

	fclose(file);
	return Z_SUCCESS;
}

enum z_Result z_read_entry_from_file(struct z_Directory* dir, FILE* file) {
	assert(dir && file);

	size_t bytes_read;
	bytes_read = fread(&dir->rank, sizeof(double), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	bytes_read = fread(&dir->last_accessed, sizeof(time_t), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;
	bytes_read = fread(&dir->path_length, sizeof(uint32_t), 1, file);
	if (bytes_read == 0 || feof(file))
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	dir->path = malloc(dir->path_length + 1);
	if (!dir->path) {
		perror("Memory allocation for z path failed");
		free(dir);
		return Z_MALLOC_ERROR;
	}

	bytes_read = fread(dir->path, sizeof(char), dir->path_length, file);
	if (bytes_read == 0)
		return Z_ZERO_BYTES_READ;
	else if (ferror(file))
		return Z_FILE_ERROR;

	dir->path[dir->path_length] = '\0';  // Null-terminate the string
	return Z_SUCCESS;
}

enum z_Result z_read_from_database_file(struct z_Database* db) {
	FILE* file = fopen(db->database_file, "rb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error opening z database file");
		if (write(STDOUT_FILENO, "Trying to create z database file.\n", 34) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			if (file)
				fclose(file);
			return Z_STDIO_ERROR;
		}

		file = fopen(db->database_file, "wb");

		if (!file || ferror(file)) {
			perror("Error creating z database file");
			if (file)
				fclose(file);
		}
		else {
			if (write(STDOUT_FILENO, "Created z database file.\n", 25) == -1) {
				perror(RED NCSH_ERROR_STDOUT RESET);
				fflush(stderr);
				if (file)
					fclose(file);
				return Z_STDIO_ERROR;
			}
		}

		if (file)
			fclose(file);
        	return Z_SUCCESS;
	}

	uint32_t number_of_entries = 0;
	size_t bytes_read = fread(&number_of_entries, sizeof(uint32_t), 1, file);
	if (number_of_entries == 0) {
		if (write(STDERR_FILENO, "Couldn't find number of entries header while trying to read z database file.\n", 77) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return Z_STDIO_ERROR;
		}
		fclose(file);
		return Z_ZERO_BYTES_READ;
	}
	else if (bytes_read == 0 || ferror(file) || feof(file)) {
		if (write(STDERR_FILENO, "Couldn't read number of entries from header while trying to read z database file.\n", 82) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return Z_STDIO_ERROR;
		}
		fclose(file);
		return Z_ZERO_BYTES_READ;
	}

	enum z_Result result;
	for (uint_fast32_t i = 0;
		i < number_of_entries && number_of_entries < Z_DATABASE_IN_MEMORY_LIMIT && !feof(file);
		++i) {
		if ((result = z_read_entry_from_file((db->dirs + i), file)) != Z_SUCCESS) {
			fclose(file);
			return result;
		}
		// printf("Rank: %f\n", (dirs + i)->rank);
		// printf("Last accessed: %ld\n", (dirs + i)->last_accessed);
		// printf("Path: %s\n", (dirs + i)->path);
	}

	fclose(file);

	db->count = number_of_entries;
	return Z_SUCCESS;
}

enum z_Result z_add_new_path_to_database(char* path, size_t path_length, struct z_Database* db) {
	assert(path && db && path_length > 1);
	if (db->count == Z_DATABASE_IN_MEMORY_LIMIT)
		return Z_FAILURE;

	db->dirs[db->count].path = malloc(path_length);
	if (!db->dirs[db->count].path)
		return Z_MALLOC_ERROR;

	memcpy(db->dirs[db->count].path, path, path_length);
	db->dirs[db->count].path_length = path_length;
	++db->dirs[db->count].rank;
	db->dirs[db->count].last_accessed = time(NULL);
	++db->count;

	return Z_SUCCESS;
}

enum z_Result z_add_new_to_database(char* path, size_t path_length, const char* cwd, size_t cwd_length, struct z_Database* db) {
	assert(db);
	if (!db || !cwd || !path || path_length == 0)
		return Z_NULL_REFERENCE;

	// printf("add to database cwd %s\n", cwd);
	// printf("add new to database path %s\n", path);

	size_t total_length = path_length + cwd_length;
	db->dirs[db->count].path = malloc(total_length);
	if (!db->dirs[db->count].path)
		return Z_MALLOC_ERROR;

	memcpy(db->dirs[db->count].path, cwd, cwd_length);
	// printf("first memcpy %s\n", db->dirs[db->count].path);
	strncat(db->dirs[db->count].path, "/", 2);
	// printf("first strcat %s\n", db->dirs[db->count].path);
	strncat(db->dirs[db->count].path, path, path_length);
	// printf("second strcat %s\n", db->dirs[db->count].path);
	strncat(db->dirs[db->count].path, "\0", 2);

	db->dirs[db->count].path_length = total_length;
	++db->dirs[db->count].rank;
	db->dirs[db->count].last_accessed = time(NULL);
	++db->count;

	return Z_SUCCESS;
}

enum z_Result z_database_file_set(struct eskilib_String config_file, struct z_Database* db) {
	#ifdef Z_TEST
	db->database_file = malloc(Z_DATABASE_FILE_LENGTH);
	if (!db->database_file)
		return Z_MALLOC_ERROR;
	memcpy(db->database_file, Z_DATABASE_FILE, Z_DATABASE_FILE_LENGTH);
	return Z_SUCCESS;
	#endif /* ifdef Z_TEST */
	if (!config_file.value || config_file.length == 0) {
		return Z_NULL_REFERENCE;
	}

	if (config_file.length + Z_DATABASE_FILE_LENGTH > NCSH_MAX_INPUT) {
		return Z_FILE_LENGTH_TOO_LARGE;
	}

	db->database_file = malloc(config_file.length + Z_DATABASE_FILE_LENGTH);
	if (!db->database_file)
		return Z_MALLOC_ERROR;

	memcpy(db->database_file, config_file.value, config_file.length);
	strncat(db->database_file, Z_DATABASE_FILE, Z_DATABASE_FILE_LENGTH);
	return Z_SUCCESS;
}

enum z_Result z_init(struct eskilib_String config_file, struct z_Database* db) {
	assert(db);
	if (!db)
		return Z_NULL_REFERENCE;

	enum z_Result result;
	if ((result = z_database_file_set(config_file, db)) != Z_SUCCESS || !db->database_file)
		return result;

	return z_read_from_database_file(db);
}

enum z_Result z_directory_matches(char* target, size_t target_length, const char* directory, struct eskilib_String* output) {
	assert(target && directory && target_length > 0);
	if (!target || target_length == 0 || !directory)
		return Z_NULL_REFERENCE;

	struct dirent* directory_entry;
	DIR* current_directory = opendir(directory);
	if (!current_directory) {
		perror("z: could not open directory");
		return Z_FAILURE;
	}

	size_t directory_length;
	size_t compare_length;
	while ((directory_entry = readdir(current_directory))) {
		directory_length = strlen(directory_entry->d_name) + 1;
		compare_length = directory_length > target_length ? directory_length : target_length;
		if (eskilib_string_equals(target, directory_entry->d_name, compare_length)) {
			output->length = directory_length;
			output->value = malloc(directory_length);
			if (!output->value)
				return Z_MALLOC_ERROR;
			eskilib_string_copy(output->value, directory_entry->d_name, directory_length);

			if ((closedir(current_directory)) == -1) {
				perror("z: could not close directory");
				return Z_FAILURE;
			}

			return Z_SUCCESS;
		}
	}

	if ((closedir(current_directory)) == -1) {
		perror("z: could not close directory");
		return Z_FAILURE;
	}
	return Z_MATCH_NOT_FOUND;
}

void z(char* target, size_t target_length, const char* cwd, struct z_Database* db) {
	// printf("z_run: %s\n", target.value);

	if (!target) {
		char* home = getenv("HOME");
		if (!home || chdir(home) == -1)
			perror("z: couldn't change directory");

		return;
	}

	assert(target && target_length > 0 && cwd && db);
	if (!cwd || !db || !target || target_length < 2)
		return;

	if (target_length == 2 && target[0] == '.') {
		if (chdir(target) == -1)
			perror("z: couldn't change directory (1)");

		return;
	}
	else if (target_length == 3 && target[0] == '.' && target[1] == '.') {
		if (chdir(target) == -1)
			perror("z: couldn't change directory (2)");

		return;
	}

	size_t cwd_length = strlen(cwd) + 1;
	struct eskilib_String output = {0};
	struct z_Directory* match = z_find_match(target, target_length, cwd, cwd_length, db);
	if (z_directory_matches(target, target_length, cwd, &output) == Z_SUCCESS) {
		// printf("dir matches %s\n", output.value);
		if (!match && z_add_new_to_database(output.value, output.length, cwd, cwd_length, db) != Z_SUCCESS) {
			if (output.value)
				free(output.value);
			return;
		}

		if (chdir(output.value) == -1) {
			if (output.value)
				free(output.value);
			if (!match) {
				perror("z: couldn't change directory (3)");
				return;
			}
		}
		else {
			if (output.value)
				free(output.value);
			return;
		}
	}

	if (match && match->path && match->path_length > 0) {
		if (chdir(match->path) == -1) {
			perror("z: couldn't change directory (4)");
		}
		return;
	}

	if (chdir(target) == -1) {
		perror("z: couldn't change directory");
		return;
	}
}

enum z_Result z_add(char* path, size_t path_length, struct z_Database* db) {
	if (!path || !db)
		return Z_NULL_REFERENCE;
	if (path_length < 2 || path[path_length - 1] != '\0')
		return Z_BAD_STRING;

	if (z_find_match_add(path, path_length, db)) {
		if (write(STDERR_FILENO, "Entry already exists in z database.\n", 36) == -1) {
			return Z_FAILURE;
		}
		return Z_SUCCESS;
	}

	if (z_add_new_path_to_database(path, path_length, db) == Z_SUCCESS) {
		if (write(STDERR_FILENO, "Added new entry to z database.\n", 31) == -1) {
			return Z_FAILURE;
		}
		return Z_SUCCESS;
	}

	if (write(STDERR_FILENO, "Error adding new entry to z database.\n", 38) == -1) {
		return Z_FAILURE;
	}
	return Z_MALLOC_ERROR;
}

void z_free(struct z_Database* db) {
	assert(db);
	if (!db)
		return;

	if (db->database_file)
		free(db->database_file);

	for (uint_fast32_t i = 0; i < db->count; ++i) {
		if (db->dirs[i].path)
			free(db->dirs[i].path);
	}
	db->count = 0;
}

enum z_Result z_exit(struct z_Database* db) {
	assert(db);
	if (!db)
		return Z_NULL_REFERENCE;

	enum z_Result result;
	if ((result = z_write_to_database_file(db)) != Z_SUCCESS) {
		if (write(STDERR_FILENO, "Error writing to z database file.\n", 34) == -1) {
			return result;
		}
		return result;
	}

	z_free(db);
	return Z_SUCCESS;
}

