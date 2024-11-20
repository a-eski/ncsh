#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

#include "z.h"
#include "../eskilib/eskilib_file.h"
#include "../eskilib/eskilib_colors.h"

#define Z_DELIMITER ':'
#define Z_DEBUG

static char* z_database_file;

void z_database_file_set(const size_t config_path_max, struct eskilib_String config_path) {
	#ifdef Z_TEST
	z_database_file = Z_DATABASE_FILE;
	return;
	#endif /* ifdef Z_TEST */
	if (config_path.value == NULL || config_path.length == 0) {
		z_database_file = NULL;
		return;
	}

	if (config_path.length + Z_DATABASE_FILE_LENGTH > config_path_max) {
		z_database_file = NULL;
		return;
	}

	z_database_file = config_path.value;

	memcpy(z_database_file, Z_DATABASE_FILE, Z_DATABASE_FILE_LENGTH);
}

enum eskilib_Result z_database_malloc(struct z_Database* database) {
	database->dirty = false;
	database->directories = malloc(sizeof(struct eskilib_String) * Z_DATABASE_IN_MEMORY_LIMIT);
	if (database->directories == NULL)
		return E_FAILURE_MALLOC;

	return E_SUCCESS;
}

enum eskilib_Result z_database_set_directory(char* buffer, int_fast32_t buffer_length, struct z_Directory* directory) {
	directory->path.length = buffer_length;
	directory->path.value = malloc(sizeof(char) * buffer_length);
	if (directory->path.value == NULL)
		return E_FAILURE_MALLOC;

	eskilib_string_copy(directory->path.value, buffer, buffer_length);

	#ifdef Z_DEBUG
	printf("Loaded: %s\n", directory->path.value);
	#endif

	return E_SUCCESS;
}

enum eskilib_Result z_database_load(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return E_FAILURE_NULL_REFERENCE;

	FILE* file;
	if (z_database_file != NULL)
		file = fopen(z_database_file, "r");
	else
		file = fopen(Z_DATABASE_FILE, "r");

	if (file == NULL) {
		if (z_database_file != NULL)
			file = fopen(z_database_file, "w");
		else
			file = fopen(Z_DATABASE_FILE, "w");

		if (file == NULL)
		{
			perror(RED "Could not load or create z database file" RESET);
			return E_FAILURE_FILE_OP;
		}
	}

	char buffer[PATH_MAX];
	int_fast32_t buffer_length = 0;
	struct z_Directory* directories = database->directories;

	for (uint_fast32_t i = 0;
		(buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF && i < Z_DATABASE_IN_MEMORY_LIMIT;
		i++) {
		if (buffer_length > 0) {
			++database->count;
			z_database_set_directory(buffer, buffer_length, directories);
			++directories;
		}
	}

	fclose(file);

	return E_SUCCESS;
}

enum eskilib_Result z_database_save(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL || z_database_file == NULL)
		return E_FAILURE_NULL_REFERENCE;

	FILE* file = fopen(z_database_file, "a");
	if (file == NULL) {
		file = fopen(z_database_file, "w");

		if (file == NULL) {
			perror(RED "z: Could not open database file to save current session" RESET);
			return E_FAILURE_FILE_OP;
		}
	}

	fclose(file);
	return E_SUCCESS;
}

enum eskilib_Result z_database_clean(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return E_FAILURE_NULL_REFERENCE;

	return E_SUCCESS;
}

enum eskilib_Result z_database_free(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return E_FAILURE_NULL_REFERENCE;

	for (uint_fast32_t i = 0; i < database->count; i++)
		free(database->directories[i].path.value);

	if (database->directories != NULL)
		free(database->directories);

	return E_SUCCESS;
}

struct eskilib_String z_database_get_match(const struct eskilib_String target, struct z_Database* database) {
	assert(target.value != NULL);

	for (uint_fast32_t i = 0; i < database->count; i++) {
		if (eskilib_string_contains(database->directories[i].path, target)) {
			return database->directories[i].path;
		}
	}

	return eskilib_String_Empty;
}

enum eskilib_Result z_database_add(const struct eskilib_String target, struct z_Database* database) {
	if (database == NULL || target.value == NULL || target.length == 0)
		return E_FAILURE_NULL_REFERENCE;

	database->dirty = true;

	uint_fast32_t length = 0;
	for (uint_fast32_t i = 0; i < database->count; i++) {
		length = database->directories[i].path.length > target.length
			? database->directories[i].path.length
			: target.length;

		if (eskilib_string_equals(database->directories[i].path.value, target.value, length)) {
			database->directories[i].rank = 1;
			database->directories[i].last_accessed = clock();
		}
	}

	database->directories[database->count].path = target;
	++database->directories[database->count].rank;
	database->directories[database->count].last_accessed = clock();
	++database->count;

	return E_SUCCESS;
}

double z_score(struct z_Directory* directory, clock_t now) {
	clock_t duration = now - directory->last_accessed;

	if (duration < Z_HOUR)
		return directory->rank * 4.0;
	else if (duration < Z_DAY)
		return directory->rank * 2.0;
	else if (duration < Z_WEEK)
		return directory->rank * 0.5;
	else
		return directory->rank * 0.25;
}

enum eskilib_Result z_begin(const size_t config_path_max, struct eskilib_String config_path, struct z_Database* database) {
	z_database_file_set(config_path_max, config_path);
	if (z_database_file == NULL)
		return E_FAILURE;

	enum eskilib_Result result = z_database_malloc(database);
	if (result != E_SUCCESS)
		return result;

	result = z_database_load(database);
	if (result != E_SUCCESS)
		return result;

	return E_SUCCESS;
}

char* z_directory_matches(const struct eskilib_String target, const char* directory) {
	struct dirent* directory_entry;
	DIR* current_directory = opendir(directory);
	if (current_directory == NULL) {
		perror("z: could not open directory");
		return NULL;
	}

	while ((directory_entry = readdir(current_directory)) != NULL) {
		if (eskilib_string_equals(target.value, directory_entry->d_name, PATH_MAX)) {
			if ((closedir(current_directory)) == -1) {
				perror("z: could not close directory");
			}

			return directory_entry->d_name;
		}
	}

	if ((closedir(current_directory)) == -1) {
		perror("z: could not close directory");
	}
	return NULL;
}

struct eskilib_String z_process (const struct eskilib_String target, const char* directory, struct z_Database* database) {
	assert(target.value != NULL && target.length > 0);
	if (target.value == NULL || target.length == 0)
		return eskilib_String_Empty;

	printf("z_run: %s\n", target.value);

	if (target.length == 1) {
		if (target.value[0] == '~')
			chdir("~");
	}
	if (target.length == 2) {
		if (target.value[0] == '.' && target.value[1] == '.')
			chdir("..");
	}

	struct eskilib_String match = z_database_get_match(target, database);

	if (match.length == 0) {
		if (z_directory_matches(target, directory) == true) {
			chdir(target.value);
			return target;
		}

		if (match.value == NULL)
			return eskilib_String_Empty;
	}

	chdir(match.value);
	return match;
}

enum eskilib_Result z_end(struct z_Database* database) {
	enum eskilib_Result result = z_database_save(database);
	if (result != E_SUCCESS)
		return result;
	result = z_database_clean(database);
	if (result != E_SUCCESS)
		return result;
	result = z_database_free(database);
	if (result != E_SUCCESS)
		return result;

	return E_SUCCESS;
}

