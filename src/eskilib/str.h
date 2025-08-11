/* Copyright eskilib (C) by Alex Eski 2024 */
/* Str.h: minimalist header lib for dealing with strings */

#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "edefines.h"
#include "../arena.h"

#define Str_Empty ((Str){.value = NULL, .length = 0})

// WARN: currently all string functions using this code incorporate null terminator in length
// TODO: fix this, use length everywhere without null terminator... .length = sizeof(str) - 1
#define Str_New_Literal(str)                                                                                           \
    (Str)                                                                                                              \
    {                                                                                                                  \
        .value = str, .length = sizeof(str)                                                                            \
    }
#define Str_New(str, len)                                                                                              \
    (Str)                                                                                                              \
    {                                                                                                                  \
        .value = (str), .length = (len)                                                                                \
    }
#define Str_Get(str)                                                                                                   \
    (Str)                                                                                                              \
    {                                                                                                                  \
        .value = (str), .length = (strlen(str) + 1)                                                                    \
    }

typedef struct {
    size_t length;
    char* value;
} Str;

/* estrcmp
 * A simple wrapper for memcmp that checks if lengths match before calling memcmp.
 */
enodiscard static inline bool estrcmp(char* restrict str, size_t str_len, char* restrict str_two, size_t str_two_len)
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

/* estrsplit
 * Split a string in the form of "str_one{splitter}str_two".
 * If the splitter is in last position, null is returned.
 * Returns: NULL if invalid input or splitter pos or an array of Strs length 2.
 */
enodiscard static inline Str* estrsplit(Str val, char splitter, Arena* restrict a)
{
    assert(a);
    if (!val.length) {
        return NULL;
    }

    Str* strs = arena_malloc(a, 2, Str);
    size_t i;
    for (i = 0; i < val.length - 2; ++i) { // -1 for null terminator, -1 for not checking last place
        if (val.value[i] == splitter) {
            strs[1].length = val.length - i - 1;
            assert(strs[1].length > 0);
            strs[1].value = arena_malloc(a, strs[1].length, char);
            memcpy(strs[1].value, val.value + i + 1, strs[1].length - 1);
            break;
        }
    }
    if (i == 0 || !strs[1].length) {
        return NULL;
    }
    ++i;

    assert(i > 0);
    strs[0].value = arena_malloc(a, i, char);
    memcpy(strs[0].value, val.value, i - 1);
    strs[0].value[i] = '\0';
    strs[0].length = i;

    return strs;
}

enodiscard static inline Str* estrjoin(Str* v, Str* v2, char joiner, Arena* restrict a)
{
    assert(v); assert(v2); assert(a);
    if (!v || !v2 || !v->length || !v2->length) {
        return NULL;
    }

    Str* str = arena_malloc(a, 1, Str);
    str->length = v->length + v2->length;
    str->value = arena_malloc(a, str->length, char);
    memcpy(str->value, v->value, v->length - 1);
    str->value[v->length - 1] = joiner;
    memcpy(str->value + v->length, v2->value, v2->length - 1);
    return str;
}

enodiscard static inline Str* estrcat(Str* restrict v, Str* restrict v2, Arena* restrict a)
{
    assert(v); assert(v2); assert(a);
    if (!v || !v2 || !v->length || !v2->length) {
        return NULL;
    }

    Str* str = arena_malloc(a, 1, Str);
    str->length = v->length + v2->length - 1;
    str->value = arena_malloc(a, str->length, char);
    memcpy(str->value, v->value, v->length - 1);
    memcpy(str->value + v->length, v2->value, v->length - 1);
    return str;
}
