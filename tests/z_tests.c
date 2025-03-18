// Copyright (c) z by Alex Eski 2024

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/eskilib/eskilib_test.h"
#include "../src/z/z.h"

#define CWD_LENGTH 528

static const struct eskilib_String config_location = { .length = 0, .value = NULL };

double z_score(struct z_Directory* const restrict directory,
               const int fzf_score,
               const time_t now);

struct z_Directory* z_match_find(char* const target,
                                 const size_t target_length,
                                 const char* const cwd,
                                 const size_t cwd_length,
                                 struct z_Database* const restrict db,
                                 struct ncsh_Arena* const scratch_arena);

enum z_Result z_database_add(const char* const path,
                             const size_t path_length,
                             const char* const cwd,
                             const size_t cwd_length,
                             struct z_Database* const restrict db,
                             struct ncsh_Arena* const arena);

// read from empty database file
void z_read_empty_database_file_test(void)
{
    remove(Z_DATABASE_FILE);
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    NCSH_ARENA_TEST_TEARDOWN;
}

// read from non-empty database
void z_read_non_empty_database_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    struct eskilib_String new_value = {.length = 5};
    new_value.value = malloc(new_value.length);
    strcpy(new_value.value, "ncsh");

    double initial_rank = db.dirs[0].rank;
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct z_Directory* match = z_match_find(new_value.value, new_value.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eskilib_assert(match != NULL);
    eskilib_assert(db.count == 1);
    eskilib_assert(match->path_length == 57);
    eskilib_assert(match->rank > initial_rank);
    eskilib_assert(memcmp(match->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eskilib_assert(match->last_accessed > 0);

    eskilib_assert(z_exit(&db) == Z_SUCCESS);
    free(new_value.value);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// add to empty database
void z_add_to_database_empty_database_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    remove(Z_DATABASE_FILE);

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    struct eskilib_String new_value = {.length = 5};
    new_value.value = malloc(new_value.length);
    strcpy(new_value.value, "ncsh");
    struct eskilib_String cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eskilib_assert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);
    eskilib_assert(db.dirs[0].path_length == 57);
    eskilib_assert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eskilib_assert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

    free(new_value.value);
    NCSH_ARENA_TEST_TEARDOWN;
}

// z add new entry
void z_add_new_entry_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    remove(Z_DATABASE_FILE);

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    eskilib_assert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);
    eskilib_assert(db.dirs[0].path_length == 52);
    eskilib_assert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52) == 0);

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
}

// z add existing entry
void z_add_existing_in_database_new_entry(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    double initial_rank = db.dirs[0].rank;
    eskilib_assert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);
    eskilib_assert(db.dirs[0].path_length == 52);
    eskilib_assert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", 52) == 0);
    eskilib_assert(db.dirs[0].rank > initial_rank);

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
}

// z add null parameters
void z_add_null_parameters(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    eskilib_assert(z_add(NULL, 0, &db, &arena) == Z_NULL_REFERENCE);
    eskilib_assert(z_add(NULL, 3, &db, &arena) == Z_NULL_REFERENCE);
    eskilib_assert(z_add("..", 3, NULL, &arena) == Z_NULL_REFERENCE);

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
}

// z add bad parameters
void z_add_bad_parameters(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    eskilib_assert(z_add(".", 0, &db, &arena) == Z_BAD_STRING);
    eskilib_assert(z_add(".", 1, &db, &arena) == Z_BAD_STRING);
    eskilib_assert(z_add("..", 1, &db, &arena) == Z_BAD_STRING);
    eskilib_assert(z_add("..", 2, &db, &arena) == Z_BAD_STRING);

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
}

void z_add_new_entry_contained_in_another_entry_but_different_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    eskilib_assert(z_add("/mnt/c/Users/Alex/source/repos/PersonalRepos", 45, &db, &arena) == Z_SUCCESS);

    eskilib_assert(db.count == 2);
    eskilib_assert(db.dirs[1].path_length == 45);

    // z_print(&db);

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
}

// find empty database
void z_match_find_empty_database_test(void)
{
    remove(Z_DATABASE_FILE);
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    struct eskilib_String target = {.value = "path", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eskilib_assert(result == NULL);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// write to empty database
void z_write_empty_database_test(void)
{
    remove(Z_DATABASE_FILE);
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    struct eskilib_String new_value = {.length = 5};
    new_value.value = malloc(new_value.length);
    strcpy(new_value.value, "ncsh");
    struct eskilib_String cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eskilib_assert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);
    eskilib_assert(db.dirs[0].path_length == 57);
    eskilib_assert(memcmp(db.dirs[0].path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eskilib_assert(db.dirs[0].rank > 0 && db.dirs[0].last_accessed > 0);

    eskilib_assert(z_exit(&db) == Z_SUCCESS);

    free(new_value.value);
    NCSH_ARENA_TEST_TEARDOWN;
}

// try add bad/empty value to database
void z_add_to_database_empty_value_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);
    struct eskilib_String cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells", .length = 52};

    eskilib_assert(z_database_add(eskilib_String_Empty.value, eskilib_String_Empty.length, cwd.value, cwd.length,
                                         &db, &arena) == Z_NULL_REFERENCE);
    eskilib_assert(db.count == 1);

    NCSH_ARENA_TEST_TEARDOWN;
}

// adds new value to non-empty database
void z_write_nonempty_database_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 1);

    double start_rank = db.dirs[0].rank;
    struct eskilib_String new_value = {.length = 9};
    new_value.value = malloc(new_value.length);
    strcpy(new_value.value, "ttytest2");
    struct eskilib_String cwd = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos", .length = 45};

    eskilib_assert(z_database_add(new_value.value, new_value.length, cwd.value, cwd.length, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);
    eskilib_assert(db.dirs[0].path_length == 57);
    eskilib_assert(db.dirs[0].rank == start_rank);
    eskilib_assert(db.dirs[1].path_length == 54);

    eskilib_assert(z_exit(&db) == Z_SUCCESS);

    free(new_value.value);
    NCSH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, exact match
void z_match_find_finds_exact_match_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    struct eskilib_String target = {.value = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", .length = 57};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eskilib_assert(result != NULL);
    eskilib_assert(result->path_length == 57);
    eskilib_assert(memcmp(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eskilib_assert(result->rank > 0 && result->last_accessed > 0);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, match
void z_match_find_finds_match_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    struct eskilib_String target = {.value = "ncsh", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eskilib_assert(result != NULL);
    eskilib_assert(result->path_length == 57);
    eskilib_assert(memcmp(result->path, "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells/ncsh", 57) == 0);
    eskilib_assert(result->rank > 0 && result->last_accessed > 0);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, no match
void z_match_find_no_match_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    struct eskilib_String target = {.value = "path", .length = 5};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }
    struct z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    if (result)
        printf("result: %s\n", result->path);
    eskilib_assert(result == NULL);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// find from non-empty database, multiple matches, takes highest score
void z_match_find_multiple_matches_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    struct eskilib_String target = {.value = "PersonalRepos", .length = 14};
    char cwd[CWD_LENGTH];
    if (!getcwd(cwd, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }
    struct z_Directory* result = z_match_find(target.value, target.length, cwd, strlen(cwd) + 1, &db, &scratch_arena);
    eskilib_assert(result != NULL);
    eskilib_assert(result->path_length == 57);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "ncsh", .length = 5};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_home_empty_target_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = eskilib_String_Empty;
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);
    eskilib_assert(strcmp(buffer_after, getenv("HOME")) == 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_no_match_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "zzz", .length = 4};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) == 0);

    if (chdir(buffer) == -1) { // remove when directory location added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_valid_subdirectory_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "tests", .length = 6};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// multiple dirs i.e. ncsh -> src/z
void z_dir_slash_dir_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }
    size_t buffer_length = strlen(buffer) + 1;
    struct eskilib_String target = {.value = "tests/test_dir", .length = 15};
    z_database_add(target.value, target.length, buffer, buffer_length, &db, &arena);

    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);
    eskilib_assert(eskilib_string_contains(buffer_after, buffer_length, "tests/test_dir", 15));

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// normal things like ..
void z_double_dot_change_directory_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 3);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "..", .length = 3};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    z_exit(&db);
    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

void z_empty_database_valid_subdirectory_change_directory_test(void)
{
    remove(Z_DATABASE_FILE);
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 0);

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "tests", .length = 6};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

// checks that when multiple entries are contained in another, it chooses the correct entry.
void z_contains_correct_match_test(void)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct z_Database db = {0};
    eskilib_assert(z_init(&config_location, &db, &arena) == Z_SUCCESS);
    eskilib_assert(db.count == 2);

    // "/mnt/c/Users/Alex/source/repos/PersonalRepos"
    // "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells"

    char buffer[CWD_LENGTH];
    char buffer_after[CWD_LENGTH]; // need to change directory back after test so next tests work

    if (!getcwd(buffer, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    struct eskilib_String target = {.value = "PersonalRepos", .length = 14};
    z(target.value, target.length, buffer, &db, &arena, scratch_arena);
    const char* path = "/mnt/c/Users/Alex/source/repos/PersonalRepos/shells";

    if (!getcwd(buffer_after, CWD_LENGTH)) {
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    eskilib_assert(strcmp(buffer, buffer_after) != 0);
    eskilib_assert(strcmp(buffer_after, path) == 0);

    if (chdir(buffer) == -1) { // remove when database file location support added
        perror("Couldn't change back to previous directory");
        NCSH_ARENA_TEST_TEARDOWN;
        NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
        eskilib_assert(false);
    }

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
}

int main(void)
{
    eskilib_test_start();

    eskilib_test_run(z_read_empty_database_file_test);
    eskilib_test_run(z_add_to_database_empty_database_test);
    eskilib_test_run(z_match_find_empty_database_test);
    eskilib_test_run(z_write_empty_database_test);
    eskilib_test_run(z_read_non_empty_database_test);
    eskilib_test_run(z_add_to_database_empty_value_test);
    eskilib_test_run(z_write_nonempty_database_test);
    eskilib_test_run(z_match_find_finds_exact_match_test);
    eskilib_test_run(z_match_find_finds_match_test);
    // eskilib_test_run(z_match_find_no_match_test);
    eskilib_test_run(z_match_find_multiple_matches_test);

#ifdef NDEBUG
    eskilib_test_run(z_change_directory_test);
#endif /* ifdef NDEBUG */
    eskilib_test_run(z_home_empty_target_change_directory_test);
    eskilib_test_run(z_no_match_change_directory_test);
    eskilib_test_run(z_valid_subdirectory_change_directory_test);
#ifdef NDEBUG
    eskilib_test_run(z_dir_slash_dir_change_directory_test);
#endif /* ifdef NDEBUG */
    eskilib_test_run(z_empty_database_valid_subdirectory_change_directory_test);

    eskilib_test_run(z_add_new_entry_test);
    eskilib_test_run(z_add_existing_in_database_new_entry);
    eskilib_test_run(z_add_null_parameters);
    eskilib_test_run(z_add_bad_parameters);
    eskilib_test_run(z_add_new_entry_contained_in_another_entry_but_different_test);
    // eskilib_test_run(z_contains_correct_match_test);

    eskilib_test_finish();

    return 0;
}
