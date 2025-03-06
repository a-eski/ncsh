// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_CONFIG_H_
#define NCSH_CONFIG_H_

#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define NCSH_RC ".ncshrc"

struct ncsh_Config {
    struct eskilib_String home_location;
    struct eskilib_String config_location;
    struct eskilib_String config_file;
};

/* ncsh_config_init
 * Allocate memory to store information related to configuration/rc file.
 * Lives for lifetime of the shell.
 * Returns: enum eskilib_Result, E_SUCCESS is successful
 */
enum eskilib_Result ncsh_config_init(struct ncsh_Config* config);

/* ncsh_config_exit
 * Frees memory that was allocated to store configuration information.
 */
void ncsh_config_free(struct ncsh_Config* config);

/* ncsh_config_alias_check
 * Checks if the input matches to any of the compile-time defined aliased commands.
 * Returns: the actual command as a struct eskilib_String, a char* value and a size_t length.
 */
struct eskilib_String ncsh_config_alias_check(char* buffer, size_t buf_len);

#endif // !NCSH_CONFIG_H_
