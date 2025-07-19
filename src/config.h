/* Copyright ncsh (C) by Alex Eski 2024 */
/* config.h: interact and deal with .ncshrc and other configurations for the shell */

#pragma once

#include "arena.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "eskilib/str.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE "ncshrc"

/* Config
 * Stores home location, config location, and full path to the config file.
 */
typedef struct {
    Str home_location;
    Str config_location;
    Str config_file;
} Config;

enum eresult config_init(Config* rst config, Arena* rst arena, Arena scratch_arena);
