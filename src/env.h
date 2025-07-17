/* Copyright ncsh (C) by Alex Eski 2025 */
/* environment.h: deal with environment variables and other things related to the environment. */

#pragma once

#include "arena.h"
#include "eskilib/str.h"

#define NCSH_PATH_VAL "PATH"
#define NCSH_PATH_VAR "$PATH"

#define NCSH_HOME_VAL "HOME"
#define NCSH_XDG_CONFIG_HOME_VAL "XDG_CONFIG_HOME"
#define NCSH_HOME_VAR "$HOME"

#define NCSH_USER_VAL "USER"
#define NCSH_USER_VAR "$USER"

void env_home_get(Str* rst output, Arena* rst arena);

Str env_path_get();

Str env_user_get();
