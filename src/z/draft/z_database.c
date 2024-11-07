#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "z_directory.h"
#include "z_database.h"
#include "../eskilib/eskilib_file.h"

#define Z_DELIMITER ':'

#define Z_DEBUG

enum z_Database_Result z_database_malloc(struct z_Database* database) {
	database->dirty = false;
	database->directories = malloc(sizeof(struct eskilib_String) * Z_DATABASE_IN_MEMORY_LIMIT);
	if (database->directories == NULL)
		return Z_FAILURE_MALLOC;

	return Z_SUCCESS;
}

enum z_Database_Result z_database_set_directory(char* buffer, int_fast32_t buffer_length, struct z_Directory* directory) {
	directory->path.length = buffer_length;
	directory->path.value = malloc(sizeof(char) * buffer_length);
	if (directory->path.value == NULL)
		return Z_FAILURE_MALLOC;

	eskilib_string_copy(directory->path.value, buffer, buffer_length);

	#ifdef Z_DEBUG
	printf("Loaded: %s\n", directory->path.value);
	#endif

	return Z_SUCCESS;
}

enum z_Database_Result z_database_load(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return Z_FAILURE_NULL_REFERENCE;

	FILE* file = fopen(Z_DATABASE_FILE, "r");
	if (file == NULL) {
		file = fopen(Z_DATABASE_FILE, "w");
		if (file == NULL)
		{
			perror("Could not load or create z database file.");
			return Z_FAILURE_FILE_OP;
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

	return Z_SUCCESS;
}

enum z_Database_Result z_database_save(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return Z_FAILURE_NULL_REFERENCE;

	return Z_SUCCESS;
}

enum z_Database_Result z_database_clean(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return Z_FAILURE_NULL_REFERENCE;

	return Z_SUCCESS;
}

enum z_Database_Result z_database_free(struct z_Database* database) {
	assert(database != NULL);
	if (database == NULL)
		return Z_FAILURE_NULL_REFERENCE;

	for (uint_fast32_t i = 0; i < database->count; i++)
		free(database->directories[i].path.value);

	if (database->directories != NULL)
		free(database->directories);

	return Z_SUCCESS;
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

enum z_Database_Result z_database_add(const struct eskilib_String target, struct z_Database* database) {
	if (database == NULL || target.value == NULL || target.length == 0)
		return Z_FAILURE_NULL_REFERENCE;

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

	return Z_SUCCESS;
}

