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

/*struct z_Directory* find_match(struct eskilib_String target, uint32_t number_of_entries, struct z_Directory* dirs) {
	assert(target.value != NULL && target.length > 0);
	assert(dirs != NULL);
	if (target.value == NULL || target.length == 0 || dirs == NULL || number_of_entries == 0)
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
}*/

struct z_Directory* find_match(char* target, size_t target_length, uint32_t number_of_entries, struct z_Directory* dirs) {
	assert(target!= NULL && target_length > 0);
	assert(dirs != NULL);
	if (target == NULL || target_length == 0 || dirs == NULL || number_of_entries == 0)
		return NULL;

	struct z_Directory* current_match = NULL;
	time_t now = time(NULL);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i) {
		if (eskilib_string_contains_s2((dirs + i)->path, (dirs + i)->path_length, target, target_length)) {
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

enum z_Result write_entry_to_file(struct z_Directory* dir, FILE* file) {
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

enum z_Result write_to_database_file(uint32_t number_of_entries, struct z_Directory* dirs) {
	if (number_of_entries == 0 || dirs == NULL)
		return Z_NULL_REFERENCE;

	FILE* file = fopen(Z_DATABASE_FILE, "wb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error writing to z database file");
		fclose(file);
		return Z_FILE_ERROR;
	}

	if (fwrite(&number_of_entries, sizeof(uint32_t), 1, file) == 0 || feof(file) || ferror(file)) {
		perror("Error writing number of entries to z database file, could not write to database file");
		fclose(file);
		return Z_FILE_ERROR;
	}

	enum z_Result result;
	for (uint_fast32_t i = 0; i < number_of_entries; ++i) {
		if ((result = write_entry_to_file((dirs + i), file)) != Z_SUCCESS) {
			fclose(file);
			return result;
		}
	}

	fclose(file);
	return Z_SUCCESS;
}

enum z_Result read_entry_from_file(struct z_Directory* dir, FILE* file) {
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

enum z_Result read_from_database_file(struct z_Database* db) {
	FILE* file = fopen(Z_DATABASE_FILE, "rb");
	if (!file || feof(file) || ferror(file)) {
		perror("Error opening z database file");
		if (write(STDOUT_FILENO, "Trying to create z database file.\n", 34) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return Z_STDIO_ERROR;
		}

		file = fopen(Z_DATABASE_FILE, "wb");

		if (!file || ferror(file)) {
			perror("Error creating z database file");
		}
		else {
			if (write(STDOUT_FILENO, "Created z database file.\n", 25) == -1) {
				perror(RED NCSH_ERROR_STDOUT RESET);
				fflush(stderr);
				return Z_STDIO_ERROR;
			}
		}

		fclose(file);
        	return Z_SUCCESS;
	}

	uint32_t number_of_entries = 0;
	size_t bytes_read = fread(&number_of_entries, sizeof(uint32_t), 1, file);
	if (number_of_entries == 0) {
		if (write(STDOUT_FILENO, "Couldn't find number of entries header while trying to read z database file.\n", 77) == -1) {
			perror(RED NCSH_ERROR_STDOUT RESET);
			fflush(stderr);
			return Z_STDIO_ERROR;
		}
		fclose(file);
		return Z_ZERO_BYTES_READ;
	}
	else if (bytes_read == 0 || ferror(file) || feof(file)) {
		if (write(STDOUT_FILENO, "Couldn't read number of entries from header while trying to read z database file.\n", 82) == -1) {
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
		if ((result = read_entry_from_file((db->dirs + i), file)) != Z_SUCCESS) {
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

/*enum z_Result add_to_database(struct eskilib_String path, struct z_Database* db) {
	assert(db != NULL);

	if (db == NULL || path.value == NULL || path.length == 0)
		return Z_NULL_REFERENCE;

	for (uint_fast32_t i = 0; i < db->count; i++) {
		if ((db->dirs + i)->path_length == path.length) {
			if (eskilib_string_equals((db->dirs + i)->path, path.value, path.length)) {
				++(db->dirs + i)->rank;
				(db->dirs + i)->last_accessed = time(NULL);
				// puts("Found existing entry");
				return Z_SUCCESS;
			}
		}
	}

	// puts("Adding new entry");
	db->dirs[db->count].path = malloc(path.length);
	if (db->dirs[db->count].path == NULL)
		return Z_MALLOC_ERROR;
	// check malloc result
	eskilib_string_copy(db->dirs[db->count].path, path.value, path.length);
	db->dirs[db->count].path_length = path.length;
	++db->dirs[db->count].rank;
	db->dirs[db->count].last_accessed = time(NULL);
	++db->count;

	return Z_SUCCESS;
}*/

enum z_Result add_to_database(char* path, size_t path_length, struct z_Database* db) {
	assert(db != NULL);

	if (db == NULL || path== NULL || path_length == 0)
		return Z_NULL_REFERENCE;

	for (uint_fast32_t i = 0; i < db->count; i++) {
		if ((db->dirs + i)->path_length == path_length) {
			if (eskilib_string_equals((db->dirs + i)->path, path, path_length)) {
				++(db->dirs + i)->rank;
				(db->dirs + i)->last_accessed = time(NULL);
				// puts("Found existing entry");
				return Z_SUCCESS;
			}
		}
	}

	// puts("Adding new entry");
	db->dirs[db->count].path = malloc(path_length);
	if (db->dirs[db->count].path == NULL)
		return Z_MALLOC_ERROR;
	// check malloc result
	eskilib_string_copy(db->dirs[db->count].path, path, path_length);
	db->dirs[db->count].path_length = path_length;
	++db->dirs[db->count].rank;
	db->dirs[db->count].last_accessed = time(NULL);
	++db->count;

	return Z_SUCCESS;
}

enum z_Result add_new_to_database(char* path, size_t path_length, const char* cwd, struct z_Database* db) {

}

enum z_Result z_init(struct z_Database* db) {
	assert(db != NULL);
	if (db == NULL)
		return Z_NULL_REFERENCE;

	return read_from_database_file(db);
}

/*enum z_Result z_directory_matches(const struct eskilib_String target, const char* directory, struct eskilib_String* output) {
	struct dirent* directory_entry;
	DIR* current_directory = opendir(directory);
	if (current_directory == NULL) {
		perror("z: could not open directory");
		return Z_FAILURE;
	}

	while ((directory_entry = readdir(current_directory)) != NULL) {
		if (eskilib_string_equals(target.value, directory_entry->d_name, PATH_MAX)) {
			output->length = strlen(directory_entry->d_name) + 1;
			output->value = malloc(output->length);
			if (output->value == NULL)
				return Z_MALLOC_ERROR;
			eskilib_string_copy(output->value, directory_entry->d_name, output->length);

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
}*/

enum z_Result z_directory_matches(char* target, const char* directory, struct eskilib_String* output) {
	struct dirent* directory_entry;
	DIR* current_directory = opendir(directory);
	if (current_directory == NULL) {
		perror("z: could not open directory");
		return Z_FAILURE;
	}

	while ((directory_entry = readdir(current_directory)) != NULL) {
		if (eskilib_string_equals(target, directory_entry->d_name, PATH_MAX)) {
			output->length = strlen(directory_entry->d_name) + 1;
			output->value = malloc(output->length);
			if (output->value == NULL)
				return Z_MALLOC_ERROR;
			eskilib_string_copy(output->value, directory_entry->d_name, output->length);

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

/*void z(const struct eskilib_String target, const char* cwd, struct z_Database* db) {
	assert(target.value != NULL && target.length > 0);
	assert(cwd != NULL);
	assert(db != NULL);

	if (target.value == NULL || target.length == 0 || cwd == NULL || db == NULL)
		return;

	// printf("z_run: %s\n", target.value);

	if (target.length == 2) {
		if (target.value[0] == '~') {
			if (chdir(getenv("HOME")) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}
		else if (target.value[0] == '.') {
			if (chdir(target.value) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}

	}
	else if (target.length == 3) {
		if (target.value[0] == '.' && target.value[1] == '.') {
			if (chdir(target.value) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}
	}

	struct z_Directory* match = find_match(target, db->count, db->dirs);

	if (match == NULL || match->path == NULL || match->path_length == 0) {
		struct eskilib_String output = {0};
		if (z_directory_matches(target, cwd, &output) == Z_SUCCESS) {
			if (add_to_database(output, db) != Z_SUCCESS) {
				if (output.value != NULL)
					free(output.value);
				return;
			}
			if (chdir(target.value) == -1) {
				perror("z: couldn't change directory");
				if (output.value != NULL)
					free(output.value);
				return;
			}
			if (output.value != NULL)
				free(output.value);
		}
		return;
	}

	if (chdir(match->path) == -1) {
		perror("z: couldn't change directory");
	}
	return;
}*/

void z(char* target, size_t target_length, const char* cwd, struct z_Database* db) {
	assert(target!= NULL && target_length > 0);
	assert(cwd != NULL);
	assert(db != NULL);

	if (target== NULL || target_length == 0 || cwd == NULL || db == NULL)
		return;

	// printf("z_run: %s\n", target.value);

	if (target_length == 2) {
		if (target[0] == '~') {
			if (chdir(getenv("HOME")) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}
		else if (target[0] == '.') {
			if (chdir(target) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}

	}
	else if (target_length == 3) {
		if (target[0] == '.' && target[1] == '.') {
			if (chdir(target) == -1) {
				perror("z: couldn't change directory");
				return;
			}
		}
	}

	struct z_Directory* match = find_match(target, target_length, db->count, db->dirs);

	if (match == NULL || match->path == NULL || match->path_length == 0) {
		struct eskilib_String output = {0};
		if (z_directory_matches(target, cwd, &output) == Z_SUCCESS) {
			printf("dir matches %s\n", output.value);
			printf("cwd %s\n", cwd);
			if (add_to_database(output.value, output.length, db) != Z_SUCCESS) {
				if (output.value)
					free(output.value);
				return;
			}

			printf("added to database %s\n", output.value);
			if (chdir(output.value) == -1) {
				perror("z: couldn't change directory");
				if (output.value)
					free(output.value);
				return;
			}
			if (output.value)
				free(output.value);
		}
		return;
	}

	if (chdir(match->path) == -1) {
		perror("z: couldn't change directory");
	}
	return;
}

void z_free(struct z_Database* db) {
	assert(db != NULL);
	if (db == NULL)
		return;

	for (uint_fast32_t i = 0; i < db->count; ++i)
		free(db->dirs[i].path);
}

void z_exit(struct z_Database* db) {
	write_to_database_file(db->count, db->dirs);
	z_free(db);
}
