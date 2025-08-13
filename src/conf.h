/* Copyright ncsh (C) by Alex Eski 2024 */
/* config.h: interact and deal with .ncshrc and other configurations for the shell */

#pragma once

#include "types.h"
#include "eskilib/eresult.h"

#define NCSH "ncsh"
#define DOT_CONFIG ".config"
#define RC_FILE "ncshrc"

enum eresult conf_init(Shell* shell);
