// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_builtins_h
#define ncsh_builtins_h

#include <stdbool.h>

#include "eskilib/eskilib_string.h"
#include "ncsh_parser.h"

bool ncsh_is_exit_command(struct ncsh_Args *args);

int_fast32_t ncsh_echo_command(struct ncsh_Args *args);

int_fast32_t ncsh_help_command(void);

int_fast32_t ncsh_cd_command(struct ncsh_Args *args);

#endif // !ncsh_builtins_h
