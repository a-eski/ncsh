// Copyright (c) ncsh by Alex Eski 2025

// Still under dev.

#ifndef NCSH_ARENA_H_
#define NCSH_ARENA_H_

#include <stddef.h>
#include <stdint.h>

int_fast8_t ncsh_arena_init(size_t size);

void* ncsh_arena_malloc(size_t size);

void ncsh_arena_exit();

#endif // !NCSH_ARENA_H_
