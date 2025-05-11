/* Copyright eskilib (C) by Alex Eski 2024 */
/* Str.h: minimalist header lib for dealing with strings */

#pragma once

#include <stddef.h>
#include <string.h>

#include "edefines.h"

#define Str_Empty ((Str){.value = NULL, .length = 0})

// WARN: currently all string functions using this code incorporate null terminator in length
// TODO: fix this, use length everywhere without null terminator... .length = sizeof(str) - 1
#define Str_New_Literal(str) (Str){.value = str, .length = sizeof(str)};
#define Str_New(str, len) (Str){.value = str, .length = len};

typedef struct {
    size_t length;
    char* value;
} Str;

/* estrcmp
 * A simple wrapper for memcmp that checks if lengths match before calling memcmp.
 */
enodiscard static inline bool estrcmp(char* rst str, size_t str_len, char* rst str_two, size_t str_two_len)
{
    if (str_len != str_two_len || !str_len) {
        return false;
    }

    return !str || !memcmp(str, str_two, str_len);
}

enodiscard static inline bool estrcmp_s(Str val, Str val2)
{
    if (val.length != val2.length || !val.length) {
        return false;
    }

    return !val.value || !memcmp(val.value, val2.value, val.length);
}
