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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "alias.h"
#include "arena.h"
#include "conf.h"
#include "debug.h"
#include "defines.h" // used for macro NCSH_MAX_INPUT
#include "env.h"
#include "eskilib/efile.h"
#include "eskilib/eresult.h"
#include "eskilib/str.h"
// #include "ttyio/ttyio.h"


[[nodiscard]]
enum eresult config_location_init(Shell* restrict shell)
{
    if (!shell) {
        return E_FAILURE_NULL_REFERENCE;
    }

    shell->config.config_location.value = arena_malloc(&shell->arena, NCSH_MAX_INPUT, char);

    char* config_original_ptr = shell->config.config_location.value;
    Str* home = env_home_get(shell->env);
    shell->config.config_location.length = home->length - 1;

    constexpr size_t config_location_len = sizeof(DOT_CONFIG) + sizeof(NCSH);
    /* first +1 is "/", second is terminating null */
    constexpr size_t config_location_folder_len = 1 + config_location_len + 1;
    if (shell->config.config_location.length + config_location_folder_len > NCSH_MAX_INPUT) {
        shell->config.config_location.value[0] = '\0';
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    memcpy(shell->config.config_location.value, home->value, home->length - 1);
    shell->config.config_location.value += home->length - 1;
    *shell->config.config_location.value = '/';
    ++shell->config.config_location.length;
    ++shell->config.config_location.value;
    memcpy(shell->config.config_location.value, DOT_CONFIG "/" NCSH, config_location_len);
    shell->config.config_location.value += config_location_len;
    *shell->config.config_location.value = '\0';
    shell->config.config_location.length += config_location_len;
    shell->config.config_location.value = (char*)config_original_ptr;

    debugf("shell->config.config_location.value: %s\n", shell->config.config_location.value);
    assert(strlen(shell->config.config_location.value) + 1 == shell->config.config_location.length);
    mkdir(shell->config.config_location.value, 0755);

    return E_SUCCESS;
}

[[nodiscard]]
enum eresult config_file_set(Shell* restrict shell)
{
    constexpr size_t rc_len = sizeof(RC_FILE);
    constexpr size_t rc_len_nt = rc_len - 1;

#if defined(NCSH_IN_PLACE)
    shell->config.config_file.value = arena_malloc(arena, rc_len, char);
    memcpy(shell->config.config_file.value, RC_FILE, rc_len_nt);
    shell->config.config_file.value[rc_len_nt] = '\0';
    shell->config.config_file.length = rc_len;
    return E_SUCCESS;
#endif

    if (!shell->config.config_location.value || !shell->config.config_location.length) {
        shell->config.config_file.value = arena_malloc(&shell->arena, rc_len, char);
        memcpy(shell->config.config_file.value, RC_FILE, rc_len_nt);
        shell->config.config_file.value[rc_len_nt] = '\0';
        shell->config.config_file.length = rc_len;

        return E_SUCCESS;
    }

    if (shell->config.config_location.length + rc_len > NCSH_MAX_INPUT) {
        shell->config.config_file.value = NULL;
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    shell->config.config_file.value =
        arena_malloc(&shell->arena, shell->config.config_location.length + rc_len, char);
    memcpy(shell->config.config_file.value,
           shell->config.config_location.value,
           shell->config.config_location.length - 1);
    memcpy(shell->config.config_file.value + shell->config.config_location.length - 1,"/" RC_FILE, rc_len);
    shell->config.config_file.length =
        shell->config.config_location.length + rc_len;
    shell->config.config_file.value[shell->config.config_file.length - 1] = '\0';
    debugf("config_file %s\n", shell->config.config_file.value);

    return E_SUCCESS;
}

/* config_path_add
 * The function which handles config items which add values to PATH.
 */
#define PATH "PATH"
#define PATH_ADD "PATH+="

void config_path_add(Str* path, char* restrict val, int len, Arena* restrict scratch)
{
    assert(val);
    assert(len > 0);
    if (len <= 0 || !path->length) {
        return;
    }

    assert(!val[len - 1]);
    debugf("trying to add %s to path\n", val);

    Str new_path = {.length = path->length + (size_t)len};
    new_path.value = arena_malloc(scratch, new_path.length, char);
    memcpy(new_path.value, path->value, path->length - 1);
    new_path.value[path->length - 1] = ':';
    memcpy(new_path.value + path->length, val, (size_t)len);
    debugf("Got new path to set %s\n", new_path.value);
}

/* config_process
 * Iterate through the .ncshrc config file and perform any actions needed.
 */
#define PATH_ADD "PATH+="
#define ALIAS_ADD "ALIAS "
void config_process(FILE* restrict file, Shell* shell, Arena* scratch)
{
    int buffer_length;
    char buffer[NCSH_MAX_INPUT] = {0};
    bool update_path = false;
    Str path_key = Str_New_Literal(NCSH_PATH_VAL);
    Str* path = env_add_or_get(shell->env, path_key);

    while ((buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF) {
        // Add to path (6 because PATH+=)
        if (buffer_length > 6 && !memcmp(buffer, PATH_ADD, sizeof(PATH_ADD) - 1)) {
            assert(buffer + 6 && *(buffer + 6));
            debugf("adding to PATH: %s\n", buffer + 6);
            config_path_add(path, buffer + 6, buffer_length - 6, scratch);
            update_path = true;
        }
        // Aliasing (aliased=alias)
        else if (buffer_length > 6 && !memcmp(buffer, ALIAS_ADD, sizeof(ALIAS_ADD) - 1)) {
            assert(buffer + 6 && *(buffer + 6));
            alias_add(buffer + 6, (size_t)(buffer_length - 6), &shell->arena);
        }

        memset(buffer, '\0', (size_t)buffer_length);
    }

    if (path->length && update_path) {
        setenv(PATH, path->value, true);
    }
}

/* config_file_load
 * Loads the .ncshrc file and processes it by calling config_process if file could be loaded.
 * Returns: enum eresult, E_SUCCESS if config file loaded or user doesn't want one.
 */
[[nodiscard]]
enum eresult config_file_load(Shell* shell, Arena* scratch)
{
    FILE* file = fopen(shell->config.config_file.value, "r");
    if (!file || ferror(file) || feof(file)) {
        /*tty_print("ncsh: would you like to create a config file '%s'? [Y/n]: ", shell->config.config_file.value);

        char character;
        if (!read(STDIN_FILENO, &character, 1)) {
            tty_perror(NCSH_ERROR_STDIN);
            return E_FAILURE;
        }

        if (character != 'y' && character != 'Y') {
            if (file) {
                fclose(file);
            }
            return E_SUCCESS;
        }*/

        file = fopen(shell->config.config_file.value, "w");
        if (!file || ferror(file) || feof(file)) {
            // tty_perror("ncsh: Could not load or create config file");
            if (file) {
                fclose(file);
            }
            return E_FAILURE_FILE_OP;
        }
        // tty_send(&tcaps.newline);
        // tty_puts("ncsh: Created " RC_FILE " config file.");
        fclose(file);
        return E_SUCCESS;
    }

    config_process(file, shell, scratch);

    fclose(file);
    return E_SUCCESS;
}

/* config_init
 * Allocate memory via the arena bump allocator to store information related to configuration/rc file.
 * Lives for lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful
 */
[[nodiscard]]
enum eresult config_init(Shell* shell, Arena scratch)
{
    assert(shell && shell->arena.start);

    enum eresult result;
    if ((result = config_location_init(shell)) != E_SUCCESS) {
        return result;
    }

    if ((result = config_file_set(shell)) != E_SUCCESS) {
        return result;
    }

    if ((result = config_file_load(shell, &scratch)) != E_SUCCESS) {
        return result;
    }

    return E_SUCCESS;
}
