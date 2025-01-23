// Copyright (c) ncsh by Alex Eski 2025

#ifndef ncsh_interpreter_h
#define ncsh_interpreter_h

#include <stdint.h>

#include "ncsh_parser.h"
#include "ncsh_types.h"

int_fast32_t ncsh_interpreter_execute(struct ncsh_Shell* shell);

int_fast32_t ncsh_interpreter_execute_noninteractive(struct ncsh_Args* args);

#endif // !ncsh_interpreter_h
