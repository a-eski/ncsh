// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_builtins_h
#define ncsh_builtins_h

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

bool ncsh_is_exit_command(struct ncsh_Args args);

uint_fast32_t ncsh_echo_command(struct ncsh_Args args);

uint_fast32_t ncsh_help_command(void);

uint_fast32_t ncsh_cd_command(struct ncsh_Args args);

#endif // !ncsh_builtins_h

