// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_debug_h
#define ncsh_debug_h

#include <stdint.h>
#include <stdio.h>
#include "ncsh_args.h"

void ncsh_debug_line(char* line, uint_fast32_t length);
void ncsh_debug_args(struct ncsh_Args args);

#endif // !ncsh_debug_h

