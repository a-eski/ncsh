// Copyright (c) ncsh by Alex Eski 2025

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"

eskilib_nodiscard enum eskilib_Result ncsh_config_home(struct ncsh_Config* config)
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

eskilib_nodiscard enum eskilib_Result ncsh_config(struct ncsh_Config* config)
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

eskilib_nodiscard enum eskilib_Result ncsh_config_file_set(struct ncsh_Config* config)
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

eskilib_nodiscard enum eskilib_Result ncsh_config_load(struct ncsh_Config* config)
{
    // ask user if they would like to create a config file

    FILE* file = fopen(config->config_file.value, "r");
    if (!file) {
        file = fopen(config->config_file.value, "w");
        if (!file) {
            perror(RED "ncsh: Could not load or create config file" RESET);
            return E_FAILURE_FILE_OP;
        }
        puts("Created " NCSH_RC " config file.");
        return E_SUCCESS;
    }

    return E_SUCCESS;
}

eskilib_nodiscard enum eskilib_Result ncsh_config_init(struct ncsh_Config* config)
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
