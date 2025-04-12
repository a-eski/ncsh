/* Copyright ncsh by Alex Eski 2024 */

/* debug.h: debug functionality for ncsh.
 * Debug functions are only active when NCSH_DEBUG is defined.
 * If not, debug functions do nothing */

#pragma once

#ifndef NCSH_DEBUG
#define debug(buf)
#define debugf(fmt, ...)
#define debug_line(buf, len, max_len)
#define debug_parser_input(buf, len)
#define debug_args(args)
#define debug_argsv(argsc, argsv)
#define debug_string(str, name)
#else /* !NCSH_DEBUG */
#include <stdio.h>
#include <stdarg.h>

#include "eskilib/estr.h"
#include "parser.h"

#define debug(buf) puts(buf)
#define debugf(fmt, ...) debugf_internal(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define debug_line(buf, len, max_len) debug_line_internal(buf, len, max_len)
#define debug_parser_input(buf, len) debug_parser_input_internal(buf, len)
#define debug_args(args) debug_args_internal(args)
#define debug_argsv(argsc, argsv) debug_argsv_internal(argsc, argsv)
#define debug_string(str, name) debug_string_internal(str, name)

static inline void debugf_internal(const char* file, int line, const char* func,
                                   const char* fmt, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

static inline void debug_line_internal(char* buffer, size_t buf_position, size_t max_buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %zu\n", buf_position);
    printf("max_buf_position %zu\n", max_buf_position);
    fflush(stdout);
}

static inline void debug_parser_input_internal(const char* buffer, size_t buf_position)
{
    printf("buffer: %s\n", buffer);
    printf("buf_position: %zu\n", buf_position);
    fflush(stdout);
}

static inline void debug_args_internal(struct Args* args)
{
    printf("args.count: %lu\n", args->count);

    for (uint_fast32_t i = 0; i < args->count; ++i) {
        printf("args.values[%lu] %s\n", i, args->values[i]);
        printf("args.ops[%lu] %d\n", i, args->ops[i]);
        printf("args.lengths[%lu] %zu\n", i, args->lengths[i]);
    }
}

static inline void debug_argsv_internal(int argc, const char** argv)
{
    for (int i = 0; i < argc; ++i) {
        printf("argv[%d]: %s", i, argv[i]);
    }
}

static inline void debug_string_internal(struct estr string, const char* name)
{
    printf("%s value: %s\n", name, string.value);
    printf("%s length: %zu\n", name, string.length);
}
#endif // !NCSH_DEBUG
