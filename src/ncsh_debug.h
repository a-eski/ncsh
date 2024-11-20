// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_debug_h
#define ncsh_debug_h

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

void ncsh_debug_line(char* buffer, uint_fast32_t buf_position, uint_fast32_t max_buf_position);
void ncsh_debug_args(struct ncsh_Args args);
void ncsh_debug_config(struct eskilib_String config_location);

#endif // !ncsh_debug_h

