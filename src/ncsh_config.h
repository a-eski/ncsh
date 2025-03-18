/* Copyright (c) ncsh by Alex Eski 2024 */

#ifndef NCSH_CONFIG_H_
#define NCSH_CONFIG_H_

#include "ncsh_arena.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define NCSH_RC ".ncshrc"

/* struct ncsh_Config
 * Stores home location, config location, and full path to the config file.
 */
struct ncsh_Config {
    struct eskilib_String home_location;
    struct eskilib_String config_location;
    struct eskilib_String config_file;
};

enum eskilib_Result ncsh_config_init(struct ncsh_Config* const restrict config,
                                     struct ncsh_Arena* const arena,
                                     struct ncsh_Arena scratch_arena);

struct eskilib_String ncsh_config_alias_check(const char* const restrict buffer,
                                              const size_t buf_len);

#endif // !NCSH_CONFIG_H_
