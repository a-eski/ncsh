/* Copyright ncsh (C) by Alex Eski 2024 */
/* config.h: interact and deal with .ncshrc and other configurations for the shell */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif /* ifndef _DEFAULT_SOURCE */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "alias.h"
#include "arena.h"
#include "config.h"
#include "debug.h"
#include "defines.h"
#include "env.h"
#include "eskilib/ecolors.h"
#include "eskilib/efile.h"
#include "eskilib/eresult.h"
#include "eskilib/str.h"

[[nodiscard]]
enum eresult config_home_init(Config* rst config, Arena* rst arena)
{
    if (!config) {
        return E_FAILURE_NULL_REFERENCE;
    }

    env_home_get(&config->home_location, arena);
    if (!config->home_location.value)
        return E_FAILURE_NOT_FOUND;

    // TODO: fix weirdness with including null terminator in count but not in other places.
    --config->home_location.length;

    debugf("config->home_location.value: %s\n", config->home_location.value);
    return E_SUCCESS;
}

[[nodiscard]]
enum eresult config_location_init(Config* rst config, Arena* rst arena)
{
    if (!config) {
        return E_FAILURE_NULL_REFERENCE;
    }

    config->config_location.value = arena_malloc(arena, NCSH_MAX_INPUT, char);

    char* config_original_ptr = config->config_location.value;
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
enum eresult config_file_set(Config* rst config, Arena* rst arena)
{
    constexpr size_t rc_len = sizeof(RC_FILE);
    constexpr size_t rc_len_nt = rc_len - 1;

#if defined(NCSH_IN_PLACE)
    config->config_file.value = arena_malloc(arena, rc_len, char);
    memcpy(config->config_file.value, RC_FILE, rc_len_nt);
    config->config_file.value[rc_len_nt] = '\0';
    config->config_file.length = rc_len;
    return E_SUCCESS;
#endif

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
Str config_path_add(Str path, char* rst val, int len, Arena* rst scratch_arena)
{
    assert(val);
    assert(len > 0);
    if (len <= 0 || !path.length) {
        return path;
    }

    assert(!val[len - 1]);
    debugf("trying to add %s to path\n", val);

    Str new_path = {.length = path.length + (size_t)len};
    new_path.value = arena_malloc(scratch_arena, new_path.length, char);
    memcpy(new_path.value, path.value, path.length - 1);
    new_path.value[path.length - 1] = ':';
    memcpy(new_path.value + path.length, val, (size_t)len);
    debugf("Got new path to set %s\n", new_path.value);
    return new_path;
}

/* config_process
 * Iterate through the .ncshrc config file and perform any actions needed.
 */
#define PATH_ADD "PATH+="
#define ALIAS_ADD "ALIAS "
void config_process(FILE* rst file, Arena* rst arena, Arena* rst scratch_arena)
{
    int buffer_length;
    char buffer[NCSH_MAX_INPUT] = {0};
    bool update_path = false;
    Str path = env_path_get();

    while ((buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF) {
        // Add to path (6 because PATH+=)
        if (buffer_length > 6 && !memcmp(buffer, PATH_ADD, sizeof(PATH_ADD) - 1)) {
            assert(buffer + 6 && *(buffer + 6));
            debugf("adding to PATH: %s\n", buffer + 6);
            path = config_path_add(path, buffer + 6, buffer_length - 6, scratch_arena);
            update_path = true;
        }
        // Aliasing
        else if (buffer_length > 6 && !memcmp(buffer, ALIAS_ADD, sizeof(ALIAS_ADD) - 1)) {
            assert(buffer + 6 && *(buffer + 6));
            alias_add(buffer + 6, (size_t)(buffer_length - 6), arena);
        }

        memset(buffer, '\0', (size_t)buffer_length);
    }

    if (path.length && update_path)
        setenv(PATH, path.value, true);
}

/* config_file_load
 * Loads the .ncshrc file and processes it by calling config_process if file could be loaded.
 * Returns: enum eresult, E_SUCCESS if config file loaded or user doesn't want one.
 */
[[nodiscard]]
enum eresult config_file_load(Config* rst config, Arena* rst arena, Arena* rst scratch_arena)
{
    FILE* file = fopen(config->config_file.value, "r");
    if (!file || ferror(file) || feof(file)) {
        printf("ncsh: would you like to create a config file '%s'? [Y/n]: ", config->config_file.value);
        fflush(stdout);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
            perror(RED NCSH_ERROR_STDIN RESET);
            return E_FAILURE;
        }

        if (character != 'y' && character != 'Y') {
            if (file)
                fclose(file);
            return E_SUCCESS;
        }

        file = fopen(config->config_file.value, "w");
        if (!file || ferror(file) || feof(file)) {
            perror(RED "ncsh: Could not load or create config file" RESET);
            if (file)
                fclose(file);
            return E_FAILURE_FILE_OP;
        }
        puts("\nCreated " RC_FILE " config file.");
        fclose(file);
        return E_SUCCESS;
    }

    config_process(file, arena, scratch_arena);

    fclose(file);
    return E_SUCCESS;
}

/* config_init
 * Allocate memory via the arena bump allocator to store information related to configuration/rc file.
 * Lives for lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful
 */
[[nodiscard]]
enum eresult config_init(Config* rst config, Arena* rst arena, Arena scratch_arena)
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

    if ((result = config_file_load(config, arena, &scratch_arena)) != E_SUCCESS) {
        return result;
    }

    return E_SUCCESS;
}
