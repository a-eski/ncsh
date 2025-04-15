/* Copyright ncsh by Alex Eski 2025 */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arena.h"
#include "config.h"
#include "defines.h"
#include "eskilib/ecolors.h"
#include "eskilib/efile.h"
#include "eskilib/eresult.h"
#include "eskilib/estr.h"

[[nodiscard]]
enum eresult config_home_init(struct Config* const restrict config, struct Arena* const arena)
{
    if (!config) {
        return E_FAILURE_NULL_REFERENCE;
    }

    char* home = getenv("XDG_CONFIG_HOME");
    if (!home) {
        home = getenv("HOME");
        if (!home) { // neither HOME or XDG_CONFIG_HOME are available
            return E_FAILURE_NOT_FOUND;
        }
    }

    config->home_location.length = strlen(home);
    config->home_location.value = arena_malloc(arena, config->home_location.length + 1, char);
    memcpy(config->home_location.value, home, config->home_location.length + 1);
    debugf("config->home_location.value: %s\n", config->home_location.value);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult config_location_init(struct Config* const restrict config, struct Arena* const arena)
{
    if (!config) {
        return E_FAILURE_NULL_REFERENCE;
    }

    config->config_location.value = arena_malloc(arena, NCSH_MAX_INPUT, char);

    const char* const config_original_ptr = config->config_location.value;
    config->config_location.length = config->home_location.length;

    constexpr size_t config_location_len = sizeof(DOT_CONFIG) + sizeof(NCSH);
    /* first +1 is "/", second is terminating null */
    constexpr size_t config_location_folder_len = 1 + config_location_len + 1;
    if (config->home_location.length + config_location_folder_len > NCSH_MAX_INPUT) {
        config->config_location.value[0] = '\0';
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    memcpy(config->config_location.value, config->home_location.value, config->home_location.length);
    config->config_location.value += config->home_location.length;
    *config->config_location.value = '/';
    ++config->config_location.length;
    ++config->config_location.value;
    memcpy(config->config_location.value, DOT_CONFIG "/" NCSH, config_location_len);
    config->config_location.value += config_location_len;
    *config->config_location.value = '\0';
    config->config_location.length += config_location_len;
    config->config_location.value = (char*)config_original_ptr;

    debugf("config->config_location.value: %s\n", config->config_location.value);
    assert(strlen(config->config_location.value) + 1 == config->config_location.length);
    mkdir(config->config_location.value, 0755);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult config_file_set(struct Config* const restrict config, struct Arena* const arena)
{
    constexpr size_t rc_len = sizeof(RC_FILE);
    constexpr size_t rc_len_nt = rc_len - 1;
    if (!config->config_location.value || !config->config_location.length) {
        config->config_file.value = arena_malloc(arena, rc_len, char);
        memcpy(config->config_file.value, RC_FILE, rc_len_nt);
        config->config_file.value[rc_len_nt] = '\0';
        config->config_file.length = rc_len;

        return E_SUCCESS;
    }

    if (config->config_location.length + rc_len > NCSH_MAX_INPUT) {
        config->config_file.value = NULL;
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    config->config_file.value = arena_malloc(arena, config->config_location.length + rc_len, char);
    memcpy(config->config_file.value, config->config_location.value, config->config_location.length - 1);
    memcpy(config->config_file.value + config->config_location.length - 1, "/" RC_FILE, rc_len);
    config->config_file.length = config->config_location.length + rc_len;
    config->config_file.value[config->config_file.length - 1] = '\0';
    debugf("config_file %s\n", config->config_file.value);

    return E_SUCCESS;
}

/* config_path_add
 * The function which handles config items which add values to PATH.
 */
#define PATH "PATH"
#define PATH_ADD "PATH+="
void config_path_add(const char* const value, const int len, struct Arena* const scratch_arena)
{
    assert(len > 0);
    if (len < 0) {
        return;
    }

    char* path = getenv("PATH");
    size_t path_len = strlen(path) + 1; // null terminator here becomes : in length calc below
    char* new_path = arena_malloc(scratch_arena, path_len + (size_t)len, char);
    memcpy(new_path, path, path_len - 1);
    new_path[path_len - 2] = ':';
    memcpy(new_path + path_len - 1, value, (size_t)len);
    debugf("Got new path to set %s\n", new_path);
    setenv(PATH, new_path, true);
}

/* config_process
 * Iterate through the .ncshrc config file and perform any actions needed.
 */
void config_process(FILE* const restrict file, struct Arena* const scratch_arena)
{
    int buffer_length;
    char buffer[NCSH_MAX_INPUT] = {0};
    while ((buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF) {
        if (buffer_length > 6 && !strncmp(buffer, PATH_ADD, sizeof(PATH_ADD) - 1)) {
            config_path_add(buffer + 6, buffer_length - 6, scratch_arena);
        }

        memset(buffer, '\0', (size_t)buffer_length);
    }
}

/* config_file_load
 * Loads the .ncshrc file and processes it by calling config_process if file could be loaded.
 * Returns: enum eresult, E_SUCCESS if config file loaded or user doesn't want one.
 */
[[nodiscard]]
enum eresult config_file_load(const struct Config* const restrict config, struct Arena* const scratch_arena)
{

    FILE* file = fopen(config->config_file.value, "r");
    if (!file || ferror(file)) {
        printf("ncsh: would you like to create a config file '%s'? [Y/n]: ", config->config_file.value);
        fflush(stdout);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
            perror(RED NCSH_ERROR_STDIN RESET);
            return E_FAILURE;
        }

        if (character != 'y' && character != 'Y') {
            return E_SUCCESS;
        }

        file = fopen(config->config_file.value, "w");
        if (!file) {
            perror(RED "ncsh: Could not load or create config file" RESET);
            return E_FAILURE_FILE_OP;
        }
        puts("\nCreated " RC_FILE " config file.");
        return E_SUCCESS;
    }

    config_process(file, scratch_arena);

    fclose(file);
    return E_SUCCESS;
}

/* config_init
 * Allocate memory via the arena bump allocator to store information related to configuration/rc file.
 * Lives for lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful
 */
[[nodiscard]]
enum eresult config_init(struct Config* const restrict config, struct Arena* const arena, struct Arena scratch_arena)
{
    assert(arena);

    enum eresult result;
    if ((result = config_home_init(config, arena)) != E_SUCCESS) {
        return result;
    }

    if ((result = config_location_init(config, arena)) != E_SUCCESS) {
        return result;
    }

    if ((result = config_file_set(config, arena)) != E_SUCCESS) {
        return result;
    }

    if ((result = config_file_load(config, &scratch_arena)) != E_SUCCESS) {
        return result;
    }

    return E_SUCCESS;
}

#define GIT "git"
#define GIT_ALIAS "g"

#define NEOVIM "nvim"
#define NEOVIM_ALIAS "n"

#define MAKE "make"
#define MAKE_ALIAS "m"

#define FD "fdfind"
#define FD_ALIAS "fd"

#define CARGO "cargo"
#define CARGO_ALIAS "c"

struct Alias {
    struct estr alias;
    struct estr actual_command;
};

/* Compile-time aliases
 */
const struct Alias aliases[] = {
    {.alias = {.length = sizeof(GIT_ALIAS), .value = GIT_ALIAS},
     .actual_command = {.length = sizeof(GIT), .value = GIT}},
    {.alias = {.length = sizeof(NEOVIM_ALIAS), .value = NEOVIM_ALIAS},
     .actual_command = {.length = sizeof(NEOVIM), .value = NEOVIM}},
    {.alias = {.length = sizeof(MAKE_ALIAS), .value = MAKE_ALIAS},
     .actual_command = {.length = sizeof(MAKE), .value = MAKE}},
    {.alias = {.length = sizeof(FD_ALIAS), .value = FD_ALIAS}, .actual_command = {.length = sizeof(FD), .value = FD}},
    {.alias = {.length = sizeof(CARGO_ALIAS), .value = CARGO_ALIAS},
     .actual_command = {.length = sizeof(CARGO), .value = CARGO}},
};

/* config_alias_check
 * Checks if the input matches to any of the compile-time defined aliased commands.
 * Returns: the actual command as a struct estr, a char* value and a size_t length.
 */
struct estr config_alias_check(const char* const restrict buffer, const size_t buf_len)
{
    if (!buffer || buf_len < 2) {
        return estr_Empty;
    }

    constexpr size_t aliases_count = sizeof(aliases) / sizeof(struct Alias);
    for (uint_fast32_t i = 0; i < aliases_count; ++i) {
        if (estrcmp_cc(buffer, buf_len, aliases[i].alias.value, aliases[i].alias.length)) {
            return aliases[i].actual_command;
        }
    }

    return estr_Empty;
}
