// Copyright (c) ncsh by Alex Eski 2025

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"
#include "eskilib/eskilib_file.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"

[[nodiscard]]
enum eskilib_Result ncsh_config_home(struct ncsh_Config* config)
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
    config->home_location.value = malloc(config->home_location.length + 1);
    if (!config->home_location.value) {
        perror(RED "ncsh: Error when allocating memory for config" RESET);
        fflush(stderr);
        return E_FAILURE_MALLOC;
    }
    memcpy(config->home_location.value, home, config->home_location.length + 1);
#ifdef NCSH_DEBUG
    printf("config->home_location.value: %s\n", config->home_location.value);
#endif /* ifdef NCSH_DEBUG */

    return E_SUCCESS;
}

[[nodiscard]]
enum eskilib_Result ncsh_config(struct ncsh_Config* config)
{
    if (!config) {
        return E_FAILURE_NULL_REFERENCE;
    }

    config->config_location.value = malloc(NCSH_MAX_INPUT);
    if (!config->config_location.value) {
        perror(RED "ncsh: Error when allocating memory for config" RESET);
        fflush(stderr);
        return E_FAILURE_MALLOC;
    }

    const char* const config_original_ptr = config->config_location.value;
    config->config_location.length = config->home_location.length;

    /* first +1 is "/", second is terminating null */
    if (config->home_location.length + 1 + sizeof(DOT_CONFIG) + sizeof(NCSH) + 1 > NCSH_MAX_INPUT) {
        config->config_location.value[0] = '\0';
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    memcpy(config->config_location.value, config->home_location.value, config->home_location.length);
    config->config_location.value += config->home_location.length;
    *config->config_location.value = '/';
    ++config->config_location.length;
    ++config->config_location.value;
    memcpy(config->config_location.value, DOT_CONFIG "/" NCSH, sizeof(DOT_CONFIG) + sizeof(NCSH));
    config->config_location.value += sizeof(DOT_CONFIG) + sizeof(NCSH);
    *config->config_location.value = '\0';
    config->config_location.length += sizeof(DOT_CONFIG) + sizeof(NCSH);

    config->config_location.value = (char*)config_original_ptr;

#ifdef NCSH_DEBUG
    printf("config->config_location.value: %s\n", config->config_location.value);
#endif /* ifdef NCSH_DEBUG */

    assert(strlen(config->config_location.value) + 1 == config->config_location.length);
    mkdir(config->config_location.value, 0755);

    return E_SUCCESS;
}

[[nodiscard]]
enum eskilib_Result ncsh_config_file_set(struct ncsh_Config* config)
{
    if (!config->config_location.value || !config->config_location.length) {
        config->config_file.value = malloc(sizeof(NCSH_RC));
        memcpy(config->config_file.value, NCSH_RC, sizeof(NCSH_RC) - 1);
        config->config_file.value[sizeof(NCSH_RC) - 1] = '\0';
        config->config_file.length = sizeof(NCSH_RC);

        return E_SUCCESS;
    }

    if (config->config_location.length + sizeof(NCSH_RC) > NCSH_MAX_INPUT) {
        config->config_file.value = NULL;
        return E_FAILURE_OVERFLOW_PROTECTION;
    }

    config->config_file.value = malloc(config->config_location.length + sizeof(NCSH_RC));
    if (!config->config_file.value) {
        return E_FAILURE_MALLOC;
    }

    memcpy(config->config_file.value, config->config_location.value, config->config_location.length - 1);
    memcpy(config->config_file.value + config->config_location.length - 1, "/" NCSH_RC, sizeof(NCSH_RC));
    config->config_file.length = config->config_location.length + sizeof(NCSH_RC);
    config->config_file.value[config->config_file.length - 1] = '\0';

#ifdef NCSH_DEBUG
    printf("config_file %s\n", config->config_file.value);
#endif /* ifdef NCSH_DEBUG */

    return E_SUCCESS;
}

#define PATH "PATH"
#define PATH_ADD "PATH+="
void ncsh_config_path_add(char* value, int len)
{
    assert(len > 0);
    if (len < 0) {
        return;
    }

    char* path = getenv("PATH");
    size_t path_len = strlen(path) + 1; // null terminator here becomes : in length calc below
    char* new_path = malloc(path_len + (size_t)len);
    memcpy(new_path, path, path_len - 1);
    new_path[path_len - 2] = ':';
    memcpy(new_path + path_len - 1, value, (size_t)len);
#ifdef NCSH_DEBUG
    printf("Got new path to set %s\n", new_path);
#endif /* ifdef NCSH_DEBUG */
    setenv(PATH, new_path, true);
    free(new_path);
}

void ncsh_config_execute(FILE* file)
{
    int buffer_length;
    char buffer[MAX_INPUT] = {0};
    while ((buffer_length = eskilib_fgets(buffer, sizeof(buffer), file)) != EOF) {
        if (buffer_length > 6 && !strncmp(buffer, PATH_ADD, sizeof(PATH_ADD) - 1)) {
            ncsh_config_path_add(buffer + 6, buffer_length - 6);
        }

        memset(buffer, '\0', (size_t)buffer_length);
    }
}

[[nodiscard]]
enum eskilib_Result ncsh_config_load(struct ncsh_Config* config)
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
        puts("\nCreated " NCSH_RC " config file.");
        return E_SUCCESS;
    }

    ncsh_config_execute(file);

    fclose(file);
    return E_SUCCESS;
}

[[nodiscard]]
enum eskilib_Result ncsh_config_init(struct ncsh_Config* config)
{
    enum eskilib_Result result;
    if ((result = ncsh_config_home(config)) != E_SUCCESS) {
        return result;
    }

    if ((result = ncsh_config(config)) != E_SUCCESS) {
        return result;
    }

    if ((result = ncsh_config_file_set(config)) != E_SUCCESS) {
        return result;
    }

    if ((result = ncsh_config_load(config)) != E_SUCCESS) {
        return result;
    }

    return E_SUCCESS;
}

void ncsh_config_free(struct ncsh_Config* config)
{
    if (config->home_location.value) {
        free(config->home_location.value);
    }
    if (config->config_location.value) {
        free(config->config_location.value);
    }
    if (config->config_file.value) {
        free(config->config_file.value);
    }
}

#define GIT "git"
#define GIT_ALIAS "g"

#define NEOVIM "nvim"
#define NEOVIM_ALIAS "n"

#define MAKE "make"
#define MAKE_ALIAS "m"

char* aliases[] = {GIT, NEOVIM, MAKE};
size_t aliases_len[] = {sizeof(GIT), sizeof(NEOVIM), sizeof(MAKE)};

char* aliased[] = {GIT_ALIAS, NEOVIM_ALIAS, MAKE_ALIAS};
size_t aliased_len[] = {sizeof(GIT_ALIAS), sizeof(NEOVIM_ALIAS), sizeof(MAKE_ALIAS)};

struct eskilib_String ncsh_config_alias_check(char* buffer, size_t buf_len)
{
    if (!buffer || buf_len < 2) {
        return eskilib_String_Empty;
    }

    for (uint_fast32_t i = 0; i < sizeof(aliased) / sizeof(char*); ++i) {
        if (eskilib_string_compare(buffer, buf_len, aliased[i], aliased_len[i])) {
            return (struct eskilib_String){.value = aliases[i], .length = aliases_len[i]};
        }
    }

    return eskilib_String_Empty;
}
