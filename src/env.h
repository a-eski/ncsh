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

void env_home_get(struct Str* restrict output, struct Arena* restrict arena);

struct Str env_path_get();
