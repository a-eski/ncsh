/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include "arena.h"
#include "eskilib/eresult.h"
#include "eskilib/estr.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE ".ncshrc"

/* struct Config
 * Stores home location, config location, and full path to the config file.
 */
struct Config {
    struct estr home_location;
    struct estr config_location;
    struct estr config_file;
};

enum eresult config_init(struct Config* const restrict config, struct Arena* const arena, struct Arena scratch_arena);

struct estr config_alias_check(const char* const restrict buffer, const size_t buf_len);
