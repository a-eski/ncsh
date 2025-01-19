// Copyright (c) ncsh by Alex Eski 2024

#ifdef NCSH_DEBUG

#ifndef ncsh_debug_h
#define ncsh_debug_h

#include <stdio.h>

#include "eskilib/eskilib_string.h"
#include "ncsh_parser.h"

static inline void ncsh_debug_line(char *buffer, size_t buf_position, size_t max_buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %llu\n", buf_position);
    printf("max_buf_position %llu\n", max_buf_position);
    fflush(stdout);
}

static inline void ncsh_debug_parser_input(const char *buffer, size_t buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %llu\n", buf_position);
    fflush(stdout);
}

static inline void ncsh_debug_args(struct ncsh_Args *args)
{
    printf("args.count: %u\n", args->count);

    for (uint_fast32_t i = 0; i < args->count; ++i)
    {
        printf("args.values[%u] %s\n", i, args->values[i]);
        printf("args.ops[%u] %d\n", i, args->ops[i]);
        printf("args.lengths[%u] %zu\n", i, args->lengths[i]);
    }
}

static inline void ncsh_debug_string(struct eskilib_String string, const char *name)
{
    printf("%s value: %s\n", name, string.value);
    printf("%s length: %llu\n", name, string.length);
}

#endif // !ncsh_debug_h

#endif // NCSH_DEBUG
