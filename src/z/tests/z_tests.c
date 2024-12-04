#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../z.h"
#include "../z_tests.h"
#include "../../eskilib/eskilib_test.h"

// read from empty database file
void z_read_from_database_file_empty_database_file_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 0);

	z_free(&db);
}

// add to empty database
void z_add_to_database_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 0);

	struct eskilib_String new_value = { .length = 5 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "ncsh");

	eskilib_assert(add_new_to_database(new_value.value, new_value.length, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", &db) == Z_SUCCESS);
	eskilib_assert(db.count == 1);
	eskilib_assert(db.dirs[0].path_length == 57);
	eskilib_assert(eskilib_string_equals(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57));
	eskilib_assert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

	z_free(&db);
	free(new_value.value);
}

// find empty database
void z_find_match_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 0);

	struct eskilib_String target = { .value = "path", .length = 5 };
	struct z_Directory* result = find_match(target.value, target.length, db.count, db.dirs);
	eskilib_assert(result == NULL);

	z_free(&db);
}

// write to empty database
void z_write_to_database_file_empty_database_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 0);

	struct eskilib_String new_value = { .length = 5 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "ncsh");

	eskilib_assert(add_new_to_database(new_value.value, new_value.length, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", &db) == Z_SUCCESS);
	eskilib_assert(db.count == 1);
	eskilib_assert(db.dirs[0].path_length == 57);
	eskilib_assert(eskilib_string_equals(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57));
	eskilib_assert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

	eskilib_assert(z_exit(&db) == Z_SUCCESS);
	free(new_value.value);
}

// read from non-empty database
void z_read_from_database_file_non_empty_database_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 1);

	struct eskilib_String new_value = { .length = 5 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "ncsh");

	double initial_rank = db.dirs[0].rank;
	struct z_Directory* match = find_match(new_value.value, new_value.length, db.count, db.dirs);
	eskilib_assert(match != NULL);
	eskilib_assert(db.count == 1);
	eskilib_assert(match->path_length == 57);
	eskilib_assert(match->rank > initial_rank);
	eskilib_assert(eskilib_string_equals(match->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57));
	eskilib_assert(match->last_accessed > 0);

	eskilib_assert(z_exit(&db) == Z_SUCCESS);
	free(new_value.value);
}

// try add bad/empty value to database
void z_add_to_database_empty_value_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 1);

	eskilib_assert(add_new_to_database(eskilib_String_Empty.value, eskilib_String_Empty.length, "a value", &db) == Z_NULL_REFERENCE);
	eskilib_assert(db.count == 1);

	z_free(&db);
}

// adds new value to non-empty database
void z_write_to_database_file_nonempty_database_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 1);

	double start_rank = db.dirs[0].rank;
	struct eskilib_String new_value = { .length = 9 };
	new_value.value = malloc(new_value.length);
	strcpy(new_value.value, "ttytest2");

	eskilib_assert(add_new_to_database(new_value.value, new_value.length, "/mnt/c/Users/Alex/source/repos/PersonalRepos", &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);
	eskilib_assert(db.dirs[0].path_length == 57);
	eskilib_assert(db.dirs[0].rank == start_rank);
	eskilib_assert(db.dirs[1].path_length == 54);

	eskilib_assert(z_exit(&db) == Z_SUCCESS);
	free(new_value.value);
}

// find from non-empty database, exact match
void z_find_match_finds_exact_match_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	struct eskilib_String target = { .value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", .length = 57 };
	struct z_Directory* result = find_match(target.value, target.length, db.count, db.dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);
	eskilib_assert(eskilib_string_equals(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57));
	eskilib_assert(result->rank > 0 && result->last_accessed > 0);

	z_free(&db);
}

// find from non-empty database, match
void z_find_match_finds_match_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	struct eskilib_String target = { .value = "ncsh", .length = 5 };
	struct z_Directory* result = find_match(target.value, target.length, db.count, db.dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);
	eskilib_assert(eskilib_string_equals(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57));
	eskilib_assert(result->rank > 0 && result->last_accessed > 0);

	z_free(&db);
}

// find from non-empty database, no match
void z_find_match_no_match_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	struct eskilib_String target = { .value = "path", .length = 5 };
	struct z_Directory* result = find_match(target.value, target.length, db.count, db.dirs);
	eskilib_assert(result == NULL);

	z_free(&db);
}

// find from non-empty database, multiple matches, takes highest score
void z_find_match_multiple_matches_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	struct eskilib_String target = { .value = "PersonalRepos", .length = 14 };
	struct z_Directory* result = find_match(target.value, target.length, db.count, db.dirs);
	eskilib_assert(result != NULL);
	eskilib_assert(result->path_length == 57);

	z_free(&db);
}

void z_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "ncsh", .length = 5 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

void z_home_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "~", .length = 2 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}

	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);
	eskilib_assert(strcmp(buffer_after, getenv("HOME")) == 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

void z_home_empty_target_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = eskilib_String_Empty;
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}

	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);
	eskilib_assert(strcmp(buffer_after, getenv("HOME")) == 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

void z_no_match_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "zzz", .length = 4 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) == 0);

	if (chdir(buffer) == -1) { // remove when directory location added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

void z_valid_subdirectory_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "tests", .length = 6 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

// multiple dirs i.e. ncsh -> src/z
void z_dir_slash_dir_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 2);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);
	struct eskilib_String target = { .value = "tests/test_dir", .length = 15 };
	add_new_to_database(target.value, target.length, buffer, &db);

	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);
	eskilib_assert(eskilib_string_contains_unsafe(buffer_after, "tests/test_dir"));

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_exit(&db);
}

// normal things like ..
void z_double_dot_change_directory_test(void) {
	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 3);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "..", .length = 3 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_exit(&db);
}

// normal things like ../..

// relative dirs like ~/.config
void z_relative_dirs_change_directory_test(void) {}

void z_empty_database_valid_subdirectory_change_directory_test(void) {
	remove(Z_DATABASE_FILE);

	struct z_Database db = {0};
	eskilib_assert(z_init(eskilib_String_Empty, &db) == Z_SUCCESS);
	eskilib_assert(db.count == 0);

	char buffer[528];
	char buffer_after[528];

	char* wd = getcwd(buffer, sizeof(buffer));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("initial dir %s\n", buffer);

	struct eskilib_String target = { .value = "tests", .length = 6 };
	z(target.value, target.length, buffer, &db);

	wd = getcwd(buffer_after, sizeof(buffer_after));
	if (wd == NULL) {
		perror("wd error");
		eskilib_assert(false);
	}
	// printf("after chdir: %s\n", buffer_after);

	eskilib_assert(strcmp(buffer, buffer_after) != 0);

	if (chdir(buffer) == -1) { // remove when database file location support added
		perror("Couldn't change back to previous directory");
		eskilib_assert(false);
	}

	z_free(&db);
}

// z add new entry
// z add existing entry
// z add null parameters

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
	eskilib_test_run("z_find_match_no_match_test", z_find_match_no_match_test);
	eskilib_test_run("z_find_match_multiple_matches_test", z_find_match_multiple_matches_test);
	eskilib_test_run("z_change_directory_test", z_change_directory_test);
	eskilib_test_run("z_home_change_directory_test", z_home_change_directory_test);
	eskilib_test_run("z_home_empty_target_change_directory_test", z_home_empty_target_change_directory_test);
	eskilib_test_run("z_no_match_change_directory_test", z_no_match_change_directory_test);
	eskilib_test_run("z_valid_subdirectory_change_directory_test", z_valid_subdirectory_change_directory_test);
	eskilib_test_run("z_dir_slash_dir_change_directory_test", z_dir_slash_dir_change_directory_test);
	eskilib_test_run("z_empty_database_valid_subdirectory_change_directory_test", z_empty_database_valid_subdirectory_change_directory_test);

	eskilib_test_finish();

	return 0;
}
