/* Copyright ncsh by Alex Eski 2024 */

/* debug.h: debug functionality for ncsh.
 * Debug functions are only active when NCSH_DEBUG is defined.
 * If not, debug functions do nothing */

#pragma once

#ifndef NCSH_DEBUG
#define debug(buf)
#define debugf(fmt, ...)
#define debug_line(buf, len, max_len)
#define debug_lexer_input(buf, len)
#define debug_lexemes(lexemes)
#define debug_argsv(argsc, argsv)
#define debug_string(str, name)
#else /* !NCSH_DEBUG */
#include <stdarg.h>
#include <stdio.h>

#include "eskilib/str.h"
#include "interpreter/lexer.h"

#define debug(buf) debug__(__FILE__, __LINE__, __func__, buf)
#define debugf(fmt, ...) debugf__(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define debug_line(buf, len, max_len) debug_line__(__FILE__, __LINE__, __func__, buf, len, max_len)
#define debug_lexer_input(buf, len) debug_lexer_input__(__FILE__, __LINE__, __func__, buf, len)
#define debug_lexemes(lexemes) debug_lexemes__(__FILE__, __LINE__, __func__, lexemes)
#define debug_argsv(argsc, argsv) debug_argsv__(__FILE__, __LINE__, __func__, argsc, argsv)
#define debug_string(str, name) debug_string__(__FILE__, __LINE__, __func__, str, name)

static inline void debug__(const char* file, const int line, const char* func, char* buffer)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "%s\n", buffer);
    fflush(stderr);
}

static inline void debugf__(const char* file, const int line, const char* func, char* fmt, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fflush(stderr);
}

static inline void debug_line__(const char* file, const int line, const char* func, char* buffer,
                                       size_t buf_position, size_t max_buf_position)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buffer: %s\n", buffer);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buf_position: %zu\n", buf_position);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "max_buf_position %zu\n", max_buf_position);
    fflush(stderr);
}

static inline void debug_lexer_input__(const char* file, const int line, const char* func, char* buffer,
                                              size_t buf_position)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buffer: %s\n", buffer);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "buf_position: %zu\n", buf_position);
    fflush(stderr);
}

static inline void debug_lexemes__(const char* file, const int line, const char* func, Lexemes* lexemes)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "lexemes->count: %lu\n", lexemes->count);

    for (size_t i = 0; i < lexemes->count; ++i) {
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "lexemes->values[%lu] %s\n", i, lexemes->vals[i]);
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "lexemes->ops[%lu] %d\n", i, lexemes->ops[i]);
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "lexemes->lengths[%lu] %zu\n", i, lexemes->lens[i]);
    }
    fflush(stderr);
}

static inline void debug_argsv__(const char* file, const int line, const char* func, int argc, char** argv)
{
    for (int i = 0; i < argc; ++i) {
        fprintf(stderr, "%s %s:%d ", file, func, line);
        fprintf(stderr, "argv[%d]: %s\n", i, argv[i]);
    }
    fflush(stderr);
}

static inline void debug_string__(const char* file, const int line, const char* func, Str string, char* name)
{
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "%s value: %s\n", name, string.value);
    fprintf(stderr, "%s %s:%d ", file, func, line);
    fprintf(stderr, "%s length: %zu\n", name, string.length);
    fflush(stderr);
}
#endif // !NCSH_DEBUG
