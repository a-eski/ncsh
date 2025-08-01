/* Copyright z (C) by Alex Eski 2024 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE // for anon enum DT_*
#endif                  /* ifndef _DEFAULT_SOURCE */

#include <assert.h>
#include <dirent.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../defines.h" // used for NCSH_MAX_INPUT
#include "../ttyio/ttyio.h"
#include "fzf.h"
#include "z.h"

double z_score(z_Directory* restrict directory, int fzf_score, time_t now)
{
    assert(directory);
    assert(fzf_score > 0);

    time_t duration = now - directory->last_accessed;

    if (duration < Z_HOUR) {
        return (directory->rank * 4.0) + fzf_score;
    }
    else if (duration < Z_DAY) {
        return (directory->rank * 2.0) + fzf_score;
    }
    else if (duration < Z_WEEK) {
        return (directory->rank * 0.5) + fzf_score;
    }
    else {
        return (directory->rank * 0.25) + fzf_score;
    }
}

bool z_match_exists(char* restrict target, size_t target_length, z_Database* restrict db)
{
    assert(db && target && target_length > 0);

    for (size_t i = 0; i < db->count; ++i) {
        if (estrcmp((db->dirs + i)->path, (db->dirs + i)->path_length, target, target_length)) {
            ++(db->dirs + i)->rank;
            (db->dirs + i)->last_accessed = time(NULL);
            return true;
        }
    }

    return false;
}

z_Directory* z_match_find(char* restrict target, size_t target_length, char* restrict cwd, size_t cwd_length, z_Database* restrict db,
                          Arena* restrict scratch_arena)
{
    assert(target && target_length && cwd && cwd_length && scratch_arena && db);
    if (!db->count || cwd_length < 2) {
        return NULL;
    }

    fzf_slab_t* slab = fzf_make_slab((fzf_slab_config_t){(size_t)1 << 6, 1 << 6}, scratch_arena);
    fzf_pattern_t* pattern = fzf_parse_pattern(target, target_length - 1, scratch_arena);
    z_Match current_match = {0};
    time_t now = time(NULL);
#ifdef Z_DEBUG
    tty_println("cwd %s, len %zu", cwd, cwd_length);
#endif

    for (size_t i = 0; i < db->count; ++i) {
        if (!estrcmp((db->dirs + i)->path, (db->dirs + i)->path_length, cwd, cwd_length)) {
            int fzf_score =
                fzf_get_score((db->dirs + i)->path, (db->dirs + i)->path_length - 1, pattern, slab, scratch_arena);
            if (!fzf_score)
                continue;

            double potential_match_z_score = z_score((db->dirs + i), fzf_score, now);
#ifdef Z_DEBUG
            tty_println("%zu %s len: %zu", i, (db->dirs + i)->path, (db->dirs + i)->path_length);
            tty_println("%s fzf_score %d", (db->dirs + i)->path, fzf_score);
            tty_println("%s z_score %f", (db->dirs + i)->path, potential_match_z_score);
#endif /* ifdef Z_DEBUG */

            if (!current_match.dir || current_match.z_score < potential_match_z_score) {
                current_match.z_score = potential_match_z_score;
                current_match.dir = (db->dirs + i);
            }
        }
    }

#ifdef Z_DEBUG
    tty_println("match %s", current_match.dir->path);
#endif /* ifdef Z_DEBUG */

    return current_match.dir;
}

enum z_Result z_write_entry(z_Directory* restrict dir, FILE* restrict file)
{
    assert(dir && file);

    size_t bytes_written;

    bytes_written = fwrite(&dir->rank, sizeof(double), 1, file);
    if (!bytes_written || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    bytes_written = fwrite(&dir->last_accessed, sizeof(time_t), 1, file);
    if (!bytes_written || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    bytes_written = fwrite(&dir->path_length, sizeof(uint32_t), 1, file);
    if (!bytes_written || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    bytes_written = fwrite(dir->path, sizeof(char), dir->path_length, file);
    if (!bytes_written) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    return Z_SUCCESS;
}

#define Z_ERROR_WRITING_TO_DB_MESSAGE "z: Error writing to z database file"

enum z_Result z_write(z_Database* restrict db)
{
    assert(db);
    if (!db) {
        return Z_NULL_REFERENCE;
    }
    if (!db->count) {
        return Z_SUCCESS;
    }

    FILE* file = fopen(db->database_file, "wb");
    if (!file || feof(file) || ferror(file)) {
        tty_perror(Z_ERROR_WRITING_TO_DB_MESSAGE);
        if (file) {
            fclose(file);
        }
        return Z_FILE_ERROR;
    }

    if (!fwrite(&db->count, sizeof(uint32_t), 1, file) || feof(file) || ferror(file)) {
        tty_perror("Error writing number of entries to z database file, could not write to database file");
        fclose(file);
        return Z_FILE_ERROR;
    }

    enum z_Result result;
    for (size_t i = 0; i < db->count; ++i) {
        if ((result = z_write_entry((db->dirs + i), file)) != Z_SUCCESS) {
            fclose(file);
            return result;
        }
    }

    fclose(file);
    return Z_SUCCESS;
}

enum z_Result z_read_entry(z_Directory* restrict dir, FILE* restrict file, Arena* restrict arena)
{
    assert(dir && file);

    size_t bytes_read;
    bytes_read = fread(&dir->rank, sizeof(double), 1, file);
    if (!bytes_read || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    bytes_read = fread(&dir->last_accessed, sizeof(time_t), 1, file);
    if (!bytes_read || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }
    bytes_read = fread(&dir->path_length, sizeof(uint32_t), 1, file);
    if (!bytes_read || feof(file)) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }
    else if (dir->path_length == 0) {
        return Z_FILE_ERROR;
    }

    dir->path = arena_malloc(arena, dir->path_length + 1, char);

    bytes_read = fread(dir->path, sizeof(char), dir->path_length, file);
    if (!bytes_read) {
        return Z_ZERO_BYTES_READ;
    }
    else if (ferror(file)) {
        return Z_FILE_ERROR;
    }

    dir->path[dir->path_length] = '\0'; // Null-terminate the string
    return Z_SUCCESS;
}

#define Z_CREATING_DB_FILE_MESSAGE "z: trying to create z database file."
#define Z_CREATED_DB_FILE "z: created z database file."
#define Z_NO_COUNT_HEADER                                                                                              \
    "z: couldn't find number of entries header while trying to read z database file. File is empty or "           \
    "corrupted."

enum z_Result z_read(z_Database* restrict db, Arena* restrict arena)
{
    FILE* file = fopen(db->database_file, "rb");

    if (!file || feof(file) || ferror(file)) {
        tty_perror("z: z database file could not be found or opened");
        tty_writeln(Z_CREATING_DB_FILE_MESSAGE, sizeof(Z_CREATING_DB_FILE_MESSAGE) - 1);

        file = fopen(db->database_file, "wb");

        if (!file || ferror(file)) {
            tty_perror("z: error creating z database file");
            if (file) {
                fclose(file);
            }
        }
        else {
            tty_writeln(Z_CREATED_DB_FILE, sizeof(Z_CREATED_DB_FILE) - 1);
        }

        if (file) {
            fclose(file);
        }
        return Z_SUCCESS;
    }

    uint32_t number_of_entries = 0;
    size_t bytes_read = fread(&number_of_entries, sizeof(uint32_t), 1, file);
    if (!number_of_entries) {
        tty_writeln(Z_NO_COUNT_HEADER, sizeof(Z_NO_COUNT_HEADER) - 1);
        fclose(file);
        return Z_SUCCESS;
    }
    else if (!bytes_read || ferror(file) || feof(file)) {
        tty_writeln(Z_NO_COUNT_HEADER, sizeof(Z_NO_COUNT_HEADER) - 1);
        fclose(file);
        return Z_SUCCESS;
    }

    enum z_Result result;
    for (uint32_t i = 0; i < number_of_entries && number_of_entries < Z_DATABASE_IN_MEMORY_LIMIT && !feof(file); ++i) {
        if ((result = z_read_entry((db->dirs + i), file, arena)) != Z_SUCCESS) {
            fclose(file);
            return result;
        }
#ifdef Z_DEBUG
        tty_println("Rank: %f", (db->dirs + i)->rank);
        tty_println("Last accessed: %ld", (db->dirs + i)->last_accessed);
        tty_println("Path: %s", (db->dirs + i)->path);
#endif /* ifdef Z_DEBUG */
    }

    fclose(file);

    db->count = number_of_entries;
    return Z_SUCCESS;
}

enum z_Result z_write_entry_new(char* restrict path, size_t path_length, z_Database* restrict db, Arena* restrict arena)
{
    assert(path && db && path_length > 1);
    assert(path[path_length - 1] == '\0');
    if (db->count == Z_DATABASE_IN_MEMORY_LIMIT) {
        return Z_FAILURE;
    }

    db->dirs[db->count].path = arena_malloc(arena, path_length, char);

    memcpy(db->dirs[db->count].path, path, path_length);
    assert(db->dirs[db->count].path[path_length - 1] == '\0');

    db->dirs[db->count].path_length = path_length;
    ++db->dirs[db->count].rank;
    db->dirs[db->count].last_accessed = time(NULL);
    ++db->count;

    return Z_SUCCESS;
}

enum z_Result z_database_add(char* restrict path, size_t path_length, char* restrict cwd, size_t cwd_length, z_Database* restrict db,
                             Arena* restrict arena)
{
    assert(db && arena);
    if (!path || !path_length) {
        return Z_NULL_REFERENCE;
    }

    if (db->count + 1 >= Z_DATABASE_IN_MEMORY_LIMIT) {
        return Z_HIT_MEMORY_LIMIT;
    }

    assert(path && path[path_length - 1] == '\0');
    assert(strlen(path) + 1 == path_length);
    assert(cwd && cwd[cwd_length - 1] == '\0');
    assert(strlen(cwd) + 1 == cwd_length);

    size_t total_length = path_length + cwd_length;
    assert(total_length > 0);

    db->dirs[db->count].path = arena_malloc(arena, total_length, char);

    memcpy(db->dirs[db->count].path, cwd, cwd_length);
    db->dirs[db->count].path[cwd_length - 1] = '/';
    memcpy(db->dirs[db->count].path + cwd_length, path, path_length);

    assert(strlen(db->dirs[db->count].path) + 1 == total_length);
    assert(db->dirs[db->count].path[total_length - 1] == '\0');

#ifdef Z_DEBUG
    tty_println("adding new value to db after memcpys %s", db->dirs[db->count].path);
#endif /* ifdef Z_DEBUG */

    db->dirs[db->count].path_length = total_length;
    ++db->dirs[db->count].rank;
    db->dirs[db->count].last_accessed = time(NULL);
    ++db->count;

    return Z_SUCCESS;
}

enum z_Result z_database_file_set([[maybe_unused]] Str* restrict config_file, z_Database* restrict db, Arena* restrict arena)
{
    constexpr size_t z_db_file_len = sizeof(Z_DATABASE_FILE);
#if defined(Z_TEST) || defined(NCSH_IN_PLACE)
    db->database_file = arena_malloc(arena, z_db_file_len, char);
    memcpy(db->database_file, Z_DATABASE_FILE, z_db_file_len);
    return Z_SUCCESS;
#else
    if (!config_file->value || !config_file->length) {
        return Z_NULL_REFERENCE;
    }

    if (config_file->length + z_db_file_len > NCSH_MAX_INPUT) {
        return Z_FILE_LENGTH_TOO_LARGE;
    }

    db->database_file = arena_malloc(arena, config_file->length + z_db_file_len, char);

    memcpy(db->database_file, config_file->value, config_file->length);
    memcpy(db->database_file + config_file->length - 1, Z_DATABASE_FILE, z_db_file_len);

#ifdef Z_DEBUG
    tty_println("db->database_file :%s", db->database_file);
#endif /* ifdef Z_DEBUG */

    return Z_SUCCESS;
#endif /* ifdef Z_TEST */
}

enum z_Result z_init(Str* restrict config_file, z_Database* restrict db, Arena* restrict arena)
{
    assert(db);
    if (!db) {
        return Z_NULL_REFERENCE;
    }

    enum z_Result result;
    if ((result = z_database_file_set(config_file, db, arena)) != Z_SUCCESS || !db->database_file) {
        return result;
    }

    return z_read(db, arena);
}

[[nodiscard]]
bool z_is_dir(struct dirent* restrict dir)
{
#ifdef _DIRENT_HAVE_D_TYPE
    if (dir->d_type != DT_UNKNOWN) {
        return dir->d_type == DT_DIR;
    }
#endif /* ifdef _DIRENT_HAVE_D_TYPE */

    struct stat sb;
    return !stat(dir->d_name, &sb) && S_ISDIR(sb.st_mode);
}

enum z_Result z_directory_match_exists(char* restrict target, size_t target_length, char* restrict cwd, Str* restrict output,
                                       Arena* restrict scratch_arena)
{
    assert(target && cwd && target_length > 0);

    struct dirent* dir;
    DIR* current_dir = opendir(cwd);
    if (!current_dir) {
        tty_perror("z: could not open directory");
        return Z_FAILURE;
    }

    size_t dir_len;
    while ((dir = readdir(current_dir))) {
        if (dir->d_name[0] == '\0' && dir->d_type) {
            continue;
        }

#ifdef _DIRENT_HAVE_D_RECLEN
        dir_len = dir->d_reclen + 1;
#else
        dir_len = strlen(dir->d_name) + 1;
#endif /* _DIRENT_HAVE_D_RECLEN */

        if (z_is_dir(dir) && estrcmp(dir->d_name, dir_len, target, target_length)) {
            output->value = arena_malloc(scratch_arena, dir_len, char);
            output->length = dir_len;
            memcpy(output->value, dir->d_name, dir_len);

            if ((closedir(current_dir)) == -1) {
                tty_perror("z: could not close directory");
                return Z_FAILURE;
            }

            return Z_SUCCESS;
        }
    }

    if ((closedir(current_dir)) == -1) {
        tty_perror("z: could not close directory");
        return Z_FAILURE;
    }

    return Z_MATCH_NOT_FOUND;
}

void z(char* restrict target, size_t target_length, char* restrict cwd, z_Database* restrict db, Arena* restrict arena, Arena scratch_arena)
{
#ifdef Z_DEBUG
    tty_println("z: %s", target);
#endif /* ifdef Z_DEBUG */

    char* home = getenv("HOME");
    if (!home) {
        tty_perror("z: couldn't get HOME from environment");
    }

    if (!target) {
        if (home) {
            if (chdir(home) == -1) {
                tty_perror("z: couldn't change directory to home");
            }
        }

        return;
    }

    assert(target_length && cwd && db && arena && scratch_arena.start);
    if (!cwd || !db || target_length < 2) {
        return;
    }
    assert(!target[target_length - 1]);
    if (target[target_length - 1]) {
        return;
    }

    if (estrcmp(target, target_length, home, strlen(home) + 1)) {
        if (chdir(home) == -1) {
            tty_perror("z: couldn't change directory to home");
        }

        return;
    }

    // handle z .
    if (target_length == 2 && target[0] == '.') {
        if (chdir(target) == -1) {
            tty_perror("z: couldn't change directory (1)");
        }

        return;
    }
    // handle z ..
    else if (target_length == 3 && target[0] == '.' && target[1] == '.') {
        if (chdir(target) == -1) {
            tty_perror("z: couldn't change directory (2)");
        }

        return;
    }

    size_t cwd_length = strlen(cwd) + 1;
    Str output = {0};
    z_Directory* match = z_match_find(target, target_length, cwd, cwd_length, db, &scratch_arena);

    if (z_directory_match_exists(target, target_length, cwd, &output, &scratch_arena) == Z_SUCCESS) {
#ifdef Z_DEBUG
        tty_println("dir matches %s", output.value);
#endif /* ifdef Z_DEBUG */

        if (chdir(output.value) == -1) {
            if (!match) {
                tty_perror("z: couldn't change directory (3)");
                return;
            }
        }

        if (!match) {
            z_database_add(output.value, output.length, cwd, cwd_length, db, arena);
            return;
        }
    }

    if (match && match->path) {
        // try to change to the match first, if that doesn't work try target
        if (chdir(match->path) == -1) {
            if (chdir(target) == -1) {
                tty_perror("z: couldn't change directory (4)");
                return;
            }
            z_database_add(target, target_length, cwd, cwd_length, db, arena);
            return;
        }

        match->last_accessed = time(NULL);
        ++match->rank;
        return;
    }

    if (chdir(target) == -1) {
        tty_perror("z: couldn't change directory");
        return;
    }

    z_database_add(target, target_length, cwd, cwd_length, db, arena);
}

#define Z_ENTRY_EXISTS_MESSAGE "z: Entry already exists in z database."
#define Z_ADDED_NEW_ENTRY_MESSAGE "z: Added new entry to z database."
#define Z_ERROR_ADDING_ENTRY_MESSAGE "z: Error adding new entry to z database."

enum z_Result z_add(char* restrict path, size_t path_length, z_Database* restrict db, Arena* restrict arena)
{
    if (!path || !db) {
        tty_fputs("z: Null value passed to z add.", stderr);
        return Z_NULL_REFERENCE;
    }
    if (path_length < 2 || path[path_length - 1] != '\0') {
        tty_fputs("z: Bad string passed to z add.", stderr);
        return Z_BAD_STRING;
    }

    if (z_match_exists(path, path_length, db)) {
        tty_writeln(Z_ENTRY_EXISTS_MESSAGE, sizeof(Z_ENTRY_EXISTS_MESSAGE) - 1);
        return Z_SUCCESS;
    }

    if (z_write_entry_new(path, path_length, db, arena) == Z_SUCCESS) {
        tty_writeln(Z_ADDED_NEW_ENTRY_MESSAGE, sizeof(Z_ADDED_NEW_ENTRY_MESSAGE) - 1);
        return Z_SUCCESS;
    }

    tty_writeln(Z_ERROR_ADDING_ENTRY_MESSAGE, sizeof(Z_ERROR_ADDING_ENTRY_MESSAGE) - 1);
    return Z_CANNOT_PROCESS;
}

void z_remove_dirs_shift(size_t offset, z_Database* restrict db)
{
    if (offset + 1 == db->count) {
        return;
    }

    for (size_t i = offset; i < db->count - 1; ++i) {
        db->dirs[i] = db->dirs[i + 1];
    }
}

#define Z_ENTRY_NOT_FOUND_MESSAGE "z: Entry could not be found in z database."
#define Z_ENTRY_REMOVED_MESSAGE "z: Removed entry from z database."
enum z_Result z_remove(char* restrict path, size_t path_length, z_Database* restrict db)
{
    assert(db);

    if (!path) {
        tty_fputs("z: Null value passed to z rm/remove.", stderr);
        return Z_NULL_REFERENCE;
    }
    if (path_length < 2 || path[path_length - 1] != '\0') {
        tty_fputs("z: Bad string passed to z rm/remove.", stderr);
        return Z_BAD_STRING;
    }

    for (size_t i = 0; i < db->count; ++i) {
        if (estrcmp((db->dirs + i)->path, (db->dirs + i)->path_length, (char*)path, path_length)) {
            (db->dirs + i)->path = NULL;
            (db->dirs + i)->path_length = 0;
            (db->dirs + i)->last_accessed = 0;
            (db->dirs + i)->rank = 0;
            z_remove_dirs_shift(i, db);
            --db->count;
            tty_writeln(Z_ENTRY_REMOVED_MESSAGE, sizeof(Z_ENTRY_REMOVED_MESSAGE) - 1);
            return Z_SUCCESS;
        }
    }

    tty_writeln(Z_ENTRY_NOT_FOUND_MESSAGE, sizeof(Z_ENTRY_NOT_FOUND_MESSAGE) - 1);
    return Z_MATCH_NOT_FOUND;
}

enum z_Result z_exit(z_Database* restrict db)
{
    assert(db);
    if (!db) {
        return Z_NULL_REFERENCE;
    }

    enum z_Result result;
    if ((result = z_write(db)) != Z_SUCCESS) {
        tty_writeln(Z_ERROR_WRITING_TO_DB_MESSAGE, sizeof(Z_ERROR_WRITING_TO_DB_MESSAGE) - 1);
        return result;
    }

    return Z_SUCCESS;
}

#define Z_PRINT_MESSAGE "z: autojump/smarter cd command implementation for ncsh."

void z_print(z_Database* restrict db)
{
    tty_color_set(TTYIO_RED_ERROR);
    tty_writeln(Z_PRINT_MESSAGE, sizeof(Z_PRINT_MESSAGE) - 1);
    tty_color_reset();
    tty_send(&tcaps.newline);

    tty_println("Number of entries in the database is currently: %zu", db->count);
    tty_send(&tcaps.newline);
    if (!db->count) {
        return;
    }

    for (size_t i = 0; i < db->count; ++i) {
        tty_println("z[%zu].path: %s", i, db->dirs[i].path);
        tty_println("z[%zu].path_length: %zu", i, db->dirs[i].path_length);
        tty_println("z[%zu].last_accessed: %zu", i, db->dirs[i].last_accessed);
        tty_println("z[%zu].rank: %f", i, db->dirs[i].rank);
        tty_send(&tcaps.newline);
    }
}

void z_count(z_Database* restrict db)
{
    tty_println("Number of entries in the database is currently: %zu", db->count);
}
