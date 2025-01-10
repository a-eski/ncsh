// Copyright (c) ncsh by Alex Eski 2025

#ifndef ncsh_arena_h
#define ncsh_arena_h

#include <stddef.h>
#include <stdint.h>

int_fast8_t ncsh_arena_init(size_t size);

void *ncsh_arena_malloc(size_t size);

void ncsh_arena_exit();

#endif // !ncsh_arena_h
