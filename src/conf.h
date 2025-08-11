/* Copyright ncsh (C) by Alex Eski 2024 */
/* config.h: interact and deal with .ncshrc and other configurations for the shell */

#pragma once

#include "arena.h"
#include "types.h"
#include "eskilib/eresult.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE "ncshrc"

enum eresult config_init(Shell* shell, Arena scratch_arena);
