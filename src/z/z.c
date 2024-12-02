#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "z.h"

double z_score(struct z_Directory* directory, time_t now) {
	assert(directory != NULL);
	if (directory == NULL)
		return 0.0;

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

struct z_Directory* find_match(struct eskilib_String target, uint32_t number_of_entries, struct z_Directory* dirs) {
	assert(target.value != NULL && target.length > 0);
	assert(dirs != NULL);
	if (number_of_entries == 0)
		return NULL;

	struct z_Directory* current_match = NULL;
	time_t now = time(NULL);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i) {
		if (eskilib_string_contains_s((dirs + i)->path, (dirs + i)->path_length, target)) {
			if (current_match == NULL ||
				z_score(current_match, now) < z_score((dirs + i), now))
				current_match = (dirs + i);
		}
	}

	if (current_match != NULL) {
		++current_match->rank;
		current_match->last_accessed = time(NULL);
		// printf("Best match: %s\n", current_match->path);
		// printf("Best match score: %f\n", current_match->rank);
	}
	return current_match;
}

void write_to_database_file(uint32_t number_of_entries, struct z_Directory* dirs) {
	if (number_of_entries == 0 || dirs == NULL)
		return;

	FILE* file = fopen(Z_DATABASE_FILE, "wb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error writing to z database file");
		fclose(file);
		return;
	}

	fwrite(&number_of_entries, sizeof(uint32_t), 1, file);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i) {
		fwrite(&dirs[i].rank, sizeof(double), 1, file);
		fwrite(&dirs[i].last_accessed, sizeof(time_t), 1, file);
		fwrite(&dirs[i].path_length, sizeof(uint32_t), 1, file);
		fwrite(dirs[i].path, sizeof(char), dirs[i].path_length, file);
	}

	fclose(file);
}

void read_entry_from_file(struct z_Directory* dir, FILE* file) {

    fread(&dir->rank, sizeof(double), 1, file);
    fread(&dir->last_accessed, sizeof(time_t), 1, file);
    fread(&dir->path_length, sizeof(uint32_t), 1, file);

    dir->path = malloc(dir->path_length + 1);
    if (!dir->path) {
        perror("Memory allocation for path failed");
        free(dir);
        return;
    }

    fread(dir->path, sizeof(char), dir->path_length, file);
    dir->path[dir->path_length] = '\0';  // Null-terminate the string
}

uint32_t read_from_database_file(struct z_Directory* dirs) {
	FILE* file = fopen(Z_DATABASE_FILE, "rb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error opening z database file");
		write(STDOUT_FILENO, "Trying to create z database file.\n", 34);
		file = fopen(Z_DATABASE_FILE, "wb");

		if (!file || ferror(file))
			perror("Error creating z database file");
		else
			write(STDOUT_FILENO, "Created z database file.\n", 25);

		fclose(file);
        	return 0;
	}

	uint32_t number_of_entries = 0;
	uint32_t bytes_read = fread(&number_of_entries, sizeof(uint32_t), 1, file);
	if (number_of_entries == 0) {
		write(STDOUT_FILENO, "Couldn't find number of entries header while trying to read z database file.\n", 77);
		fclose(file);
		return 0;
	}
	else if (bytes_read == 0 || ferror(file) || feof(file)) {
		write(STDOUT_FILENO, "Couldn't read number of entries from header while trying to read z database file.\n", 82);
		fclose(file);
		return 0;
	}

	for (uint_fast32_t i = 0;
		i < number_of_entries && number_of_entries < Z_DATABASE_IN_MEMORY_LIMIT && !feof(file);
		++i) {
		read_entry_from_file((dirs + i), file);
		// printf("Rank: %f\n", (dirs + i)->rank);
		// printf("Last accessed: %ld\n", (dirs + i)->last_accessed);
		// printf("Path: %s\n", (dirs + i)->path);
	}

	fclose(file);

	return number_of_entries;
}

uint32_t add_to_database(struct eskilib_String path, uint32_t number_of_entries, struct z_Directory* dirs) {
	if (path.value == NULL || path.length == 0)
		return number_of_entries;

	for (uint_fast32_t i = 0; i < number_of_entries; i++) {
		if ((dirs + i)->path_length == path.length) {
			if (eskilib_string_equals((dirs + i)->path, path.value, path.length)) {
				++(dirs + i)->rank;
				(dirs + i)->last_accessed = time(NULL);
				// puts("Found existing entry");
				return number_of_entries;
			}
		}
	}

	// puts("Adding new entry");
	dirs[number_of_entries].path = malloc(path.length);
	eskilib_string_copy(dirs[number_of_entries].path, path.value, path.length);
	dirs[number_of_entries].path_length = path.length;
	++dirs[number_of_entries].rank;
	dirs[number_of_entries].last_accessed = time(NULL);
	return ++number_of_entries;
}

void z_init(struct z_Database* db) {
	db->count = read_from_database_file(db->dirs);
}

void z(const struct eskilib_String target, const char* directory, struct z_Database* db) {
	assert(target.value != NULL && target.length > 0);
	assert(directory != NULL);
	assert(db != NULL);

	if (target.value == NULL || target.length == 0)
		return;
	if (directory == NULL)
		return;
	if (db == NULL)
		return;

	printf("z_run: %s\n", target.value);

	if (target.length == 2) {
		if (target.value[0] == '~' || target.value[0] == '.') {
			chdir(target.value);
		}

	}
	else if (target.length == 3) {
		if (target.value[0] == '.' && target.value[1] == '.')
			chdir(target.value);
	}

	struct z_Directory* match = find_match(target, db->count, db->dirs);

	if (match->path_length == 0) {
		return;
	}

	chdir(match->path);
	return;
}

void z_free(struct z_Database* db) {
	for (uint_fast32_t i = 0; i < db->count; ++i)
		free(db->dirs[i].path);
}
