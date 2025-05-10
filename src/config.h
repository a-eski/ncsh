/* Copyright ncsh (C) by Alex Eski 2024 */
/* config.h: interact and deal with .ncshrc and other configurations for the shell */

#pragma once

#include "arena.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "eskilib/str.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE ".ncshrc"

/* struct Config
 * Stores home location, config location, and full path to the config file.
 */
struct Config {
    struct Str home_location;
    struct Str config_location;
    struct Str config_file;
};

enum eresult config_init(struct Config* restrict config, struct Arena* restrict arena,
		Arena scratch_arena);

struct Str config_alias_check(char* restrict buffer, size_t buf_len);
