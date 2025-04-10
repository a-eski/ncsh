/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#ifdef NCSH_DEBUG

#ifndef NCSH_DEBUG_H_
#define NCSH_DEBUG_H_

#include <stdio.h>

#include "eskilib/eskilib_string.h"
#include "parser.h"

static inline void debug_line(char* buffer, size_t buf_position, size_t max_buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %zu\n", buf_position);
    printf("max_buf_position %zu\n", max_buf_position);
    fflush(stdout);
}

static inline void debug_parser_input(const char* buffer, size_t buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %zu\n", buf_position);
    fflush(stdout);
}

static inline void debug_args(struct Args* args)
{
    printf("args.count: %lu\n", args->count);

    for (uint_fast32_t i = 0; i < args->count; ++i) {
        printf("args.values[%lu] %s\n", i, args->values[i]);
        printf("args.ops[%lu] %d\n", i, args->ops[i]);
        printf("args.lengths[%lu] %zu\n", i, args->lengths[i]);
    }
}

static inline void debug_argsv(int argc, const char** argv)
{
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s", i, argv[i]);
    }
}

static inline void debug_string(struct eskilib_String string, const char* name)
{
    printf("%s value: %s\n", name, string.value);
    printf("%s length: %zu\n", name, string.length);
}

#endif // !NCSH_DEBUG_H_

#endif // NCSH_DEBUG
