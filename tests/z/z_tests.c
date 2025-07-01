/* Copyright (c) z by Alex Eski 2024 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../src/eskilib/etest.h"
#include "../../src/z/z.h"
#include "../lib/arena_test_helper.h"

#define CWD_LENGTH 528

static Str config_location = {.length = 0, .value = NULL};

double z_score(z_Directory* restrict directory, int fzf_score, time_t now);

z_Directory* z_match_find(char* restrict target, size_t target_length, char* restrict cwd, size_t cwd_length,
                          z_Database* restrict db, Arena* restrict scratch_arena);

enum z_Result z_database_add(char* restrict path, size_t path_length, char* restrict cwd, size_t cwd_length,
                             z_Database* restrict db, Arena* restrict arena);

// read from empty database file
void z_read_empty_database_file_test()
{
    remove(Z_DATABASE_FILE);
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    ARENA_TEST_TEARDOWN;
}

// read from non-empty database
void z_read_non_empty_database_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    Str new_value = {.length = 5};
    new_value.value = arena_malloc(&arena, new_value.length, char);
    strcpy(new_value.value, "ncsh");

    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_Directory* match = z_match_find(new_value.value, new_value.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eassert(match != NULL);
    eassert(db.count == 1);
    eassert(match->path_length == 57);
    eassert(memcmp(match->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eassert(match->last_accessed > 0);

    eassert(z_exit(&db) == Z_SUCCESS);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// add to empty database
void z_add_to_database_empty_database_test()
{
    ARENA_TEST_SETUP;
    remove(Z_DATABASE_FILE);

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    Str new_value = {.length = 5};
    new_value.value = arena_malloc(&arena, new_value.length, char);
    strcpy(new_value.value, "ncsh");
    Str cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eassert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);
    eassert(db.dirs[0].path_length == 57);
    eassert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eassert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

    ARENA_TEST_TEARDOWN;
}

// z add new entry
void z_add_new_entry_test()
{
    ARENA_TEST_SETUP;
    remove(Z_DATABASE_FILE);

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    eassert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);
    eassert(db.dirs[0].path_length == 52);
    eassert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52) == 0);

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
}

// z add existing entry
void z_add_existing_in_database_new_entry()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    double initial_rank = db.dirs[0].rank;
    eassert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);
    eassert(db.dirs[0].path_length == 52);
    eassert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52) == 0);
    eassert(db.dirs[0].rank > initial_rank);

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
}

// z add null parameters
void z_add_null_parameters()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    eassert(z_add(NULL, 0, &db, &arena) == Z_NULL_REFERENCE);
    eassert(z_add(NULL, 3, &db, &arena) == Z_NULL_REFERENCE);
    eassert(z_add("..", 3, NULL, &arena) == Z_NULL_REFERENCE);

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
}

// z add bad parameters
void z_add_bad_parameters()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    eassert(z_add(".", 0, &db, &arena) == Z_BAD_STRING);
    eassert(z_add(".", 1, &db, &arena) == Z_BAD_STRING);
    eassert(z_add("..", 1, &db, &arena) == Z_BAD_STRING);
    eassert(z_add("..", 2, &db, &arena) == Z_BAD_STRING);

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
}

void z_add_new_entry_contained_in_another_entry_but_different_test()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    eassert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos", 45, &db, &arena) == Z_SUCCESS);

    eassert(db.count == 2);
    eassert(db.dirs[1].path_length == 45);

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
}

// find empty database
void z_match_find_empty_database_test()
{
    remove(Z_DATABASE_FILE);
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    Str target = {.value = "path", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eassert(result == NULL);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// write to empty database
void z_write_empty_database_test()
{
    remove(Z_DATABASE_FILE);
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    Str new_value = {.length = 5};
    new_value.value = arena_malloc(&arena, new_value.length, char);
    strcpy(new_value.value, "ncsh");
    Str cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eassert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);
    eassert(db.dirs[0].path_length == 57);
    eassert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eassert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

    eassert(z_exit(&db) == Z_SUCCESS);

    ARENA_TEST_TEARDOWN;
}

// try add bad/empty value to database
void z_add_to_database_empty_value_test()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);
    Str cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eassert(z_database_add(Str_Empty.value, Str_Empty.length, cwd.value, cwd.length, &db, &arena) == Z_NULL_REFERENCE);
    eassert(db.count == 1);

    ARENA_TEST_TEARDOWN;
}

// adds new value to non-empty database
void z_write_nonempty_database_test()
{
    ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 1);

    double start_rank = db.dirs[0].rank;
    Str new_value = {.length = 9};
    new_value.value = arena_malloc(&arena, new_value.length, char);
    strcpy(new_value.value, "ttytest2");
    Str cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos", .length = 45};

    eassert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);
    eassert(db.dirs[0].path_length == 57);
    eassert(db.dirs[0].rank == start_rank);
    eassert(db.dirs[1].path_length == 54);

    eassert(z_exit(&db) == Z_SUCCESS);

    ARENA_TEST_TEARDOWN;
}

// find from non-empty database, exact match
void z_match_find_finds_exact_match_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    Str target = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", .length = 57};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eassert(result != NULL);
    eassert(result->path_length == 57);
    eassert(memcmp(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eassert(result->rank > 0 && result->last_accessed > 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, match
void z_match_find_finds_match_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    Str target = {.value = "ncsh", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eassert(result != NULL);
    eassert(result->path_length == 57);
    eassert(memcmp(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eassert(result->rank > 0 && result->last_accessed > 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, no match
void z_match_find_no_match_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    Str target = {.value = "path", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }
    z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    if (result)
        printf("result: %s\n", result->path);
    eassert(result == NULL);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, multiple matches, takes highest score
void z_match_find_multiple_matches_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    Str target = {.value = "PersonalRepos", .length = 14};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        eassert(false);
    }
    z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eassert(result != NULL);
    eassert(result->path_length == 57);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "ncsh", .length = 5};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after));

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_home_empty_target_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = Str_Empty;
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) != 0);
    eassert(strcmp(buffer_after, getenv("HOME")) == 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_no_match_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "zzz", .length = 4};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) == 0);

    if (chdir(buffer) == -1) { // remove when directory location added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_valid_subdirectory_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "tests", .length = 6};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// multiple dirs i.e. ncsh -> src/z
void z_dir_slash_dir_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }
    size_t buffer_length = strlen(buffer) + 1;
    Str target = {.value = "tests/test_dir", .length = 15};
    z_database_add(target.value, target.length, buffer, buffer_length, &db, &arena);

    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) != 0);
    eassert(!strstr(buffer_after, "tests/test_dir"));

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// normal things like ..
void z_double_dot_change_directory_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 3);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "..", .length = 3};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    z_exit(&db);
    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_empty_database_valid_subdirectory_change_directory_test()
{
    remove(Z_DATABASE_FILE);
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 0);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "tests", .length = 6};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

// checks that when multiple entries are contained in another, it chooses the correct entry.
void z_contains_correct_match_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    // "/mnt/c/Users/Alex/source/repos/PersonalRepos"
    // "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells"

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    Str target = {.value = "PersonalRepos", .length = 14};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);
    char* path = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells";

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    eassert(strcmp(buffer, buffer_after));
    eassert(!strcmp(buffer_after, path));

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        ARENA_TEST_TEARDOWN;
        SCRATCH_ARENA_TEST_TEARDOWN;
        eassert(false);
    }

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_crashing_input_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    z_Database db = {0};
    eassert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eassert(db.count == 2);

    char* buffer = arena_malloc(&arena, CWD_LENGTH, char);
    Str target = {.value = "", .length = sizeof("")};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    // not crashing is a test passed

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(z_read_empty_database_file_test);
    etest_run(z_add_to_database_empty_database_test);
    etest_run(z_match_find_empty_database_test);
    etest_run(z_write_empty_database_test);
    etest_run(z_read_non_empty_database_test);
    etest_run(z_add_to_database_empty_value_test);
    etest_run(z_write_nonempty_database_test);
    etest_run(z_match_find_finds_exact_match_test);
    etest_run(z_match_find_finds_match_test);
    etest_run(z_match_find_no_match_test);
    etest_run(z_match_find_multiple_matches_test);

    etest_run(z_change_directory_test);
    etest_run(z_home_empty_target_change_directory_test);
    etest_run(z_no_match_change_directory_test);
    etest_run(z_valid_subdirectory_change_directory_test);
#ifdef NDEBUG
    etest_run(z_dir_slash_dir_change_directory_test);
#endif /* ifdef NDEBUG */
    etest_run(z_empty_database_valid_subdirectory_change_directory_test);

    etest_run(z_add_new_entry_test);
    etest_run(z_add_existing_in_database_new_entry);
    etest_run(z_add_null_parameters);
    etest_run(z_add_bad_parameters);
    etest_run(z_add_new_entry_contained_in_another_entry_but_different_test);
    etest_run(z_contains_correct_match_test);
    etest_run(z_crashing_input_test);

    etest_finish();

    remove(Z_DATABASE_FILE);

    return 0;
}
