/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <readline/ncsh_arena.h>
#include <readline/ncsh_string.h>

#include "eskilib/eskilib_result.h"

/* struct ncsh_Config
 * Stores home location, config location, and full path to the config file.
 */
struct ncsh_Config {
    struct ncsh_String home_location;
    struct ncsh_String config_location;
    struct ncsh_String config_file;
};

enum eskilib_Result ncsh_config_init(struct ncsh_Config* const restrict config,
                                     struct ncsh_Arena* const arena,
                                     struct ncsh_Arena scratch_arena);

struct ncsh_String ncsh_config_alias_check(const char* const restrict buffer,
                                              const size_t buf_len);
