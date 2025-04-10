/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include "arena.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE ".ncshrc"

/* struct Config
 * Stores home location, config location, and full path to the config file.
 */
struct Config {
    struct eskilib_String home_location;
    struct eskilib_String config_location;
    struct eskilib_String config_file;
};

enum eskilib_Result config_init(struct Config* const restrict config, struct Arena* const arena,
                                struct Arena scratch_arena);

struct eskilib_String config_alias_check(const char* const restrict buffer, const size_t buf_len);
