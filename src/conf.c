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
enum eresult conf_location_init(Shell* restrict shell)
{
    assert(shell); assert(shell->env);
    if (!shell) {
        return E_FAILURE_NULL_REFERENCE;
    }

    Str* home = env_home_get(shell->env);

    constexpr size_t location_len = sizeof(DOT_CONFIG) + sizeof(NCSH) + 1 + 1;
    if (home->length + location_len > PATH_MAX) {
        shell->config.location.value[0] = '\0';
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    shell->config.location = *estrjoin(home, &Str_New_Literal(DOT_CONFIG "/" NCSH), '/', &shell->arena);
    mkdir(shell->config.location.value, 0755);

    debugf("config location: %s\n", shell->config.location.value);
    return E_SUCCESS;
}

[[nodiscard]]
enum eresult conf_file_set(Shell* restrict shell)
{

    Str rc_file = Str_New_Literal(RC_FILE);
#if defined(NCSH_IN_PLACE)
    shell->config.file = *estrdup(&rc_file, &shell->arena);
    return E_SUCCESS;
#endif

    if (!shell->config.location.value || !shell->config.location.length) {
        shell->config.file = *estrdup(&rc_file, &shell->arena);
        return E_SUCCESS;
    }

    if (shell->config.location.length + rc_file.length > NCSH_MAX_INPUT) {
        shell->config.file.value = NULL;
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    shell->config.file = *estrjoin(&shell->config.location, &rc_file, '/', &shell->arena);

    debugf("config file %s\n", shell->config.file.value);
    return E_SUCCESS;
}

/* conf_process
 * Iterate through the .ncshrc config file and perform any actions needed.
 */
#define PATH_ADD "PATH+="
#define ALIAS_ADD "ALIAS "
void conf_process(FILE* restrict file, Shell* restrict shell, Arena* restrict scratch)
{
    int buffer_length;
    char buffer[NCSH_MAX_INPUT] = {0};
    Str path_key = Str_New_Literal(NCSH_PATH_VAL);
    Str* path = env_add_or_get(shell->env, path_key);
    Str_Builder* sb = sb_new(scratch);
    sb_add(path, sb, scratch);

    while ((buffer_length = efgets(buffer, sizeof(buffer), file)) != EOF) {
        // Add to path (6 because PATH+=)
        if (buffer_length > 6 && !memcmp(buffer, PATH_ADD, sizeof(PATH_ADD) - 1)) {
            assert(buffer + 6 && *(buffer + 6)); assert(strlen(buffer + 6) + 1 == (size_t)(buffer_length - 6));
            sb_add(&Str_New(buffer + 6, (size_t)(buffer_length - 6)), sb, scratch);
        }
        // Aliasing (aliased=alias)
        else if (buffer_length > 6 && !memcmp(buffer, ALIAS_ADD, sizeof(ALIAS_ADD) - 1)) {
            alias_add(Str_New(buffer + 6, (size_t)(buffer_length - 6)), &shell->arena);
        }

        memset(buffer, '\0', (size_t)buffer_length);
    }

    if (sb->n > 1) {
        *path = *sb_to_joined_str(sb, ':', &shell->arena);
    }
}

/* conf_file_load
 * Loads the .ncshrc file and processes it by calling conf_process if file could be loaded.
 * Returns: enum eresult, E_SUCCESS if config file loaded or user doesn't want one.
 */
[[nodiscard]]
enum eresult conf_file_load(Shell* restrict shell, Arena* restrict scratch)
{
    FILE* file = fopen(shell->config.file.value, "r");
    if (!file || ferror(file) || feof(file)) {
        /*tty_print("ncsh: would you like to create a config file '%s'? [Y/n]: ", shell->config.file.value);

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

        file = fopen(shell->config.file.value, "w");
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

    conf_process(file, shell, scratch);

    fclose(file);
    return E_SUCCESS;
}

/* conf_init
 * Allocate memory via the arena bump allocator to store information related to configuration/rc file.
 * Lives for lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful
 */
[[nodiscard]]
enum eresult conf_init(Shell* restrict shell, Arena scratch)
{
    assert(shell); assert(shell->arena.start);

    enum eresult result;
    if ((result = conf_location_init(shell)) != E_SUCCESS) {
        debug("failed loading config location");
        return result;
    }

    if ((result = conf_file_set(shell)) != E_SUCCESS) {
        debug("failed setting config file");
        return result;
    }

    if ((result = conf_file_load(shell, &scratch)) != E_SUCCESS) {
        debug("failed loading config file");
        return result;
    }

    return E_SUCCESS;
}
