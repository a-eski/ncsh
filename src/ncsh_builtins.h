// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_builtins_h
#define ncsh_builtins_h

#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_string.h"
#include "ncsh_parser.h"

int_fast32_t ncsh_builtins_exit_command(struct ncsh_Args *args);

int_fast32_t ncsh_builtins_echo_command(struct ncsh_Args *args);

int_fast32_t ncsh_builtins_help_command(struct ncsh_Args *args);

int_fast32_t ncsh_builtins_cd_command(struct ncsh_Args *args);

int_fast32_t ncsh_builtins_set_command(struct ncsh_Args *args);

#endif // !ncsh_builtins_h

