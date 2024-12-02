#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../z.h"
#include "../z_tests.h"
#include "../../eskilib/eskilib_test.h"

// read from empty database file
void z_read_from_database_file_empty_database_file_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 0);
}

// add to empty database
void z_add_to_database_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 0);

	struct eskilib_String new_value = { .length = 57 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh");
	number_of_entries = add_to_database(new_value, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 1);
	eskilib_assert(dirs[0].path_length == 57);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
	free(new_value.value);
}

// find empty database
void z_find_match_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 0);

	struct eskilib_String target = { .value = "path", .length = 5 };
	struct z_Directory* result = find_match(target, number_of_entries, dirs);
	eskilib_assert(result == NULL);
}

// write to empty database
void z_write_to_database_file_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 0);

	struct eskilib_String new_value = { .length = 57 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh");
	number_of_entries = add_to_database(new_value, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 1);
	eskilib_assert(dirs[0].path_length == 57);

	write_to_database_file(number_of_entries, dirs);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
	free(new_value.value);
}

// read from non-empty database
void z_read_from_database_file_non_empty_database_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 1);

	struct eskilib_String new_value = { .length = 57 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh");
	number_of_entries = add_to_database(new_value, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 1);
	eskilib_assert(dirs[0].path_length == 57);

	write_to_database_file(number_of_entries, dirs);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
	free(new_value.value);
}

// try add bad/empty value to database
void z_add_to_database_empty_value_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 1);

	number_of_entries = add_to_database(eskilib_String_Empty, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 1);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
}

// adds new value to non-empty database
void z_write_to_database_file_nonempty_database_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 1);

	double start_rank = dirs[0].rank;
	struct eskilib_String new_value = { .length = 54 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "/mnt/c/Users/Alex/source/repos/PersonalRepos/ttytest2");
	number_of_entries = add_to_database(new_value, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 2);
	eskilib_assert(dirs[0].path_length == 57);
	eskilib_assert(dirs[0].rank == start_rank);
	eskilib_assert(dirs[1].path_length == 54);

	write_to_database_file(number_of_entries, dirs);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
	free(new_value.value);
}

// find from non-empty database, exact match
void z_find_match_finds_exact_match_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 2);

	struct eskilib_String target = { .value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", .length = 57 };
	struct z_Directory* result = find_match(target, number_of_entries, dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
}

// find from non-empty database, match
void z_find_match_finds_match_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 2);

	struct eskilib_String target = { .value = "ncsh", .length = 5 };
	struct z_Directory* result = find_match(target, number_of_entries, dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
}

// add duplicate to non-empty database
void z_add_to_database_duplicate_entry_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 2);

	double start_rank = dirs[0].rank;
	struct eskilib_String new_value = { .length = 57 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh");
	number_of_entries = add_to_database(new_value, number_of_entries, dirs);
	eskilib_assert(number_of_entries == 2);
	eskilib_assert(dirs[0].path_length == 57);
	eskilib_assert(dirs[0].rank > start_rank);

	write_to_database_file(number_of_entries, dirs);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
	free(new_value.value);
}

// find from non-empty database, no match
void z_find_match_no_match_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 2);

	struct eskilib_String target = { .value = "path", .length = 5 };
	struct z_Directory* result = find_match(target, number_of_entries, dirs);
	eskilib_assert(result == NULL);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
}

// find from non-empty database, multiple matches, takes highest score
void z_find_match_multiple_matches_test(void) {
	struct z_Directory dirs[Z_DATABASE_IN_MEMORY_LIMIT] = {0};
	uint32_t number_of_entries = read_from_database_file(dirs);
	eskilib_assert(number_of_entries == 2);

	struct eskilib_String target = { .value = "PersonalRepos", .length = 14 };
	struct z_Directory* result = find_match(target, number_of_entries, dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);

	for (uint_fast32_t i = 0; i < number_of_entries; ++i)
		free(dirs[i].path);
}

void z_database_test(void) {
	struct z_Database db = {0};
	z_init(&db);
	eskilib_assert(db.count == 2);

	struct eskilib_String target = { .value = "PersonalRepos", .length = 14 };
	struct z_Directory* result = find_match(target, db.count, db.dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);

	z_free(&db);
}

int main(void) {
	eskilib_test_start();

	eskilib_test_run("z_read_from_database_file_empty_database_file_test", z_read_from_database_file_empty_database_file_test);
	eskilib_test_run("z_add_to_database_empty_database_test", z_add_to_database_empty_database_test);
	eskilib_test_run("z_find_match_empty_database_test", z_find_match_empty_database_test);
	eskilib_test_run("z_write_to_database_file_empty_database_test", z_write_to_database_file_empty_database_test);
	eskilib_test_run("z_read_from_database_file_non_empty_database_test", z_read_from_database_file_non_empty_database_test);
	eskilib_test_run("z_add_to_database_empty_value_test", z_add_to_database_empty_value_test);
	eskilib_test_run("z_write_to_database_file_nonempty_database_test", z_write_to_database_file_nonempty_database_test);
	eskilib_test_run("z_find_match_finds_exact_match_test", z_find_match_finds_exact_match_test);
	eskilib_test_run("z_find_match_finds_match_test", z_find_match_finds_match_test);
	eskilib_test_run("z_add_to_database_duplicate_entry_test", z_add_to_database_duplicate_entry_test);
	eskilib_test_run("z_find_match_no_match_test", z_find_match_no_match_test);
	eskilib_test_run("z_find_match_multiple_matches_test", z_find_match_multiple_matches_test);
	eskilib_test_run("z_database_test", z_database_test);

	eskilib_test_finish();

	return 0;
}
