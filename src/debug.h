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
#include <stdarg.h>
#include <stdio.h>

#include "eskilib/estr.h"
#include "parser.h"

#define debug(buf) puts(buf)
#define debugf(fmt, ...) debugf_internal(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define debug_line(buf, len, max_len) debug_line_internal(__FILE__, __LINE__, __func__, buf, len, max_len)
#define debug_parser_input(buf, len) debug_parser_input_internal(__FILE__, __LINE__, __func__, buf, len)
#define debug_args(args) debug_args_internal(__FILE__, __LINE__, __func__, args)
#define debug_argsv(argsc, argsv) debug_argsv_internal(__FILE__, __LINE__, __func__, argsc, argsv)
#define debug_string(str, name) debug_string_internal(__FILE__, __LINE__, __func__, str, name)

static inline void debugf_internal(const char* file, int line, const char* func, const char* fmt, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

static inline void debug_line_internal(const char* file, int line, const char* func, char* buffer, size_t buf_position, size_t max_buf_position)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buffer: %s\n", buffer);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buf_position: %zu\n", buf_position);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "max_buf_position %zu\n", max_buf_position);
    fflush(stderr);

}

static inline void debug_parser_input_internal(const char* file, int line, const char* func, const char* buffer, size_t buf_position)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buffer: %s\n", buffer);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buf_position: %zu\n", buf_position);
    fflush(stderr);
}

static inline void debug_args_internal(const char* file, int line, const char* func, struct Args* args)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "args.count: %lu\n", args->count);

    for (uint_fast32_t i = 0; i < args->count; ++i) {
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "args.values[%lu] %s\n", i, args->values[i]);
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "args.ops[%lu] %d\n", i, args->ops[i]);
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "args.lengths[%lu] %zu\n", i, args->lengths[i]);
    }
    fflush(stderr);
}

static inline void debug_argsv_internal(const char* file, int line, const char* func, int argc, const char** argv)
{
    for (int i = 0; i < argc; ++i) {
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    }
    fflush(stderr);
}

static inline void debug_string_internal(const char* file, int line, const char* func, struct estr string, const char* name)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "%s value: %s\n", name, string.value);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "%s length: %zu\n", name, string.length);
    fflush(stderr);
}
#endif // !NCSH_DEBUG
