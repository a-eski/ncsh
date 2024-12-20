// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_debug_h
#define ncsh_debug_h

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

void ncsh_debug_line(char* buffer, size_t buf_position, size_t max_buf_position);

void ncsh_debug_args(struct ncsh_Args args);

void ncsh_debug_string(struct eskilib_String string, const char* name);

#endif // !ncsh_debug_h

