// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_builtins_h
#define ncsh_builtins_h

#include <stdbool.h>
#include <stdint.h>

#include "eskilib/eskilib_string.h"
#include "ncsh_parser.h"

int_fast32_t ncsh_builtins_exit(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_echo(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_help(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_cd(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_pwd(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_kill(struct ncsh_Args* args);

int_fast32_t ncsh_builtins_set(struct ncsh_Args* args);

#endif // !ncsh_builtins_h
