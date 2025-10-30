/* Copyright eskilib (C) by Alex Eski 2024 */
/* INFO: str.h: minimalist header only lib for dealing with strings */
/* WARN: currently all string functions incorporate null terminator in length and all Str are null-terminated. */

#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "edefines.h"
#include "../arena.h"

#define Str_Empty ((Str){.value = NULL, .length = 0})

#define Str_Lit(str)                                                                                           \
    (Str)                                                                                                              \
    {                                                                                                                  \
        .value = (str), .length = (sizeof(str))                                                                        \
    }
#define Str(str, len)                                                                                              \
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
enodiscard static inline bool estrcmp(Str v, Str v2)
{
    if (v.length != v2.length || !v.length) {
        return false;
    }

    return !v.value || !memcmp(v.value, v2.value, v.length);
}

enodiscard static inline bool estrcmp_a(char* restrict str, size_t str_len, char* restrict str_two, size_t str_two_len)
{
    if (str_len != str_two_len || !str_len) {
        return false;
    }

    return !str || !memcmp(str, str_two, str_len);
}

enodiscard static inline bool estrcmp_s(Str v, char* restrict v2, size_t v2_len)
{
    if (v.length != v2_len || !v.length) {
        return false;
    }

    return !v.value || !memcmp(v.value, v2, v.length);
}

/* estrsplit
 * Split a string in the form of "str_one{splitter}str_two".
 * If the splitter is in last position, null is returned.
 * Returns: NULL if invalid input or splitter pos or an array of Strs length 2.
 */
enodiscard static inline Str* estrsplit(Str val, char splitter, Arena* restrict a)
{
    assert(a);
    if (!val.length || !val.value) {
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

/* estrjoin
 *  Join 2 strings into a new one allocated in the arena and separated by the joiner character.
 */
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

    assert(strlen(str->value) + 1 == str->length);
    assert(str->value[str->length - 1] == '\0');

    return str;
}

/* estrcat
 * Allocate a new string in the arena and concatenate v2 to v.
*/
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
    memcpy(str->value + v->length - 1, v2->value, v2->length - 1);
    return str;
}

/* estridx
 *  Return the index of the first occurence of char c in Str v.
 */
enodiscard static inline ssize_t estridx(Str* v, char c)
{
    assert(v); assert(v->value);

    ssize_t idx;
    for (idx = 0; idx < (ssize_t)v->length; ++idx) {
        if (v->value[idx] == c)
            return idx;
    }
    return -1;
}

/* estrtoarr
 * Pass in an array of Str's with size n and get back a null terminated array of char*.
 */
enodiscard static inline char** estrtoarr(Str* strs, size_t n, Arena* restrict a)
{
    char** buffer = arena_malloc(a, n, char*);
    size_t i;
    for (i = 0; i < n; ++i) {
        buffer[i] = strs[i].value;
    }
    buffer[i] = NULL;
    return buffer;
}

static inline void estrtrim(Str* v)
{
    if (v->length <= 2) {
        return;
    }

    size_t i = v->length - 2;
    while (i > 0 && v->value[i] == ' ') {
        v->value[i--] = '\0';
    }
    v->length = i + 2;

    assert(v->value[v->length - 1] == '\0');
}

enodiscard static inline Str* estrnew(char* v, Arena* restrict a)
{
    Str* rv = arena_malloc(a, 1, Str);
    rv->length = strlen(v) + 1;
    rv->value = arena_malloc(a, rv->length, char);
    memcpy(rv->value, v, rv->length - 1);
    return rv;
}

enodiscard static inline Str* estrdup(Str* v, Arena* restrict a)
{
    Str* rv = arena_malloc(a, 1, Str);
    rv->value = arena_malloc(a, v->length, char);
    memcpy(rv->value, v->value, v->length - 1);
    rv->length = v->length;

    assert(rv->value[rv->length - 1] == '\0');

    return rv;
}

static inline void estrset(Str* restrict out, Str* restrict in, Arena* restrict a)
{
    out->value = arena_malloc(a, in->length, char);
    memcpy(out->value, in->value, in->length - 1);
    out->length = in->length;
}

typedef struct Str_Builder {
    size_t n;
    size_t c;
    Str* strs;
} Str_Builder;

#ifndef SB_START_N
#   define SB_START_N 10
#endif

enodiscard static inline Str_Builder* sb_new(Arena* restrict a)
{
    Str_Builder* sb = arena_malloc(a, 1, Str_Builder);
    sb->strs = arena_malloc(a, SB_START_N, Str);
    sb->n = 0;
    sb->c = SB_START_N;
    return sb;
}

[[maybe_unused]]
static void sb_add(Str* restrict v, Str_Builder* restrict sb, Arena* restrict a)
{
    assert(v->value); assert(v->length > 0); assert(*v->value); assert(strlen(v->value) + 1 == v->length);

    if (sb->n >= sb->c - 1) {
        size_t new_c = sb->c * 2;
        sb->strs = arena_realloc(a, new_c, Str, sb->strs, sb->c);
        sb->c = new_c;
    }

    sb->strs[sb->n++] = *v;
}

enodiscard static inline Str* sb_to_str(Str_Builder* restrict sb, Arena* restrict a)
{
    size_t n = 0;
    for (size_t i = 0; i < sb->n; ++i) {
        n += sb->strs[i].length - 1;
    }
    ++n;

    Str* rv = arena_malloc(a, 1, Str);
    rv->length = n;
    rv->value = arena_malloc(a, n, char);
    size_t pos = 0;
    for (size_t i = 0; i < sb->n; ++i) {
        memcpy(rv->value + pos, sb->strs[i].value, sb->strs[i].length - 1);
        pos += sb->strs[i].length - 1;
    }

    return rv;
}

enodiscard static inline Str* sb_to_joined_str(Str_Builder* restrict sb, char joiner, Arena* restrict a)
{
    size_t n = 0;
    for (size_t i = 0; i < sb->n; ++i) {
        n += sb->strs[i].length - 1;
    }
    ++n;

    Str* rv = arena_malloc(a, 1, Str);
    rv->value = arena_malloc(a, n, char);
    size_t pos = 0;
    for (size_t i = 0; i < sb->n - 1; ++i) {
        memcpy(rv->value + pos, sb->strs[i].value, sb->strs[i].length - 1);
        pos += sb->strs[i].length;
        rv->value[pos - 1] = joiner;
    }
    memcpy(rv->value + pos, sb->strs[sb->n - 1].value, sb->strs[sb->n - 1].length - 1);
    pos += sb->strs[sb->n - 1].length;
    rv->value[pos - 1] = '\0';
    rv->length = pos;

    return rv;
}

/* estrsjoin
 *  Join n strings into a new one allocated in the arena and separated by the joiner character.
 */
enodiscard static inline Str* estrsjoin(Str* v, size_t n, char joiner, Arena* restrict a)
{
    assert(v); assert(n); assert(a);
    if (!v || !n || !v->length) {
        return NULL;
    }

    if (n == 2) {
        return estrjoin(&v[0], &v[1], joiner, a);
    }

    Str_Builder* sb = sb_new(a);

    for (size_t i = 0; i < n; ++i) {
        sb_add(&v[i], sb, a);
    }

    Str* str = sb_to_joined_str(sb, joiner, a);

    assert(strlen(str->value) + 1 == str->length);
    assert(str->value[str->length - 1] == '\0');

    return str;
}

// start with non decimal numbers
// TODO: work on a solution for parsing out integers
// TODO: work on a solution for checking then parsing out decimals
enodiscard static inline bool estrisnum(Str s)
{
    if (s.length < 2) {
        return false;
    }

    if (s.value[0] != '-' && !(s.value[0] >= '0' && s.value[0] <= '9'))
        return false;

    for (size_t i = 1; i < s.length - 1; ++i) {
        if (!(s.value[i] >= '0' && s.value[i] <= '9')) {
            return false;
        }
    }

    return true;

    /*char* dot = strstr(s.value, ".");
    char* comma = strstr(s.value, ",");

    char* endptr;
    if (!dot && !comma) {
        if (s.length < 1 + floor(log10(INT_MAX))) {
            strtoimax(s.value, &endptr, 10);
        }
        else if (s.length < 1 + floor(log10(LONG_MAX))) {
            strtol(s.value, &endptr, 10);
        }
        else if (s.length < 1 + floor(log10(LLONG_MAX))) {
            strtoll(s.value, &endptr, 10);
        }
        else if (s.length < 1 + floor(log10(ULLONG_MAX))) {
            strtoull(s.value, &endptr, 10);
        }

        if (endptr == s.value || *endptr != 0 || errno == ERANGE)
            return false;

        return true;
    }

    return false;*/
}

typedef struct {
    enum {
        N_INT,
        N_DBL
    } type;
    union {
        int i;
        double d;
    } value;
} Num;

// 0 is ASCII 48, 9 is ASCII 57
enodiscard static inline int ctoi(char c)
{
    assert((int)c > 47 && (int)c < 58);
    return (int)c - 48;
}

enodiscard static inline Num estrtonum(Str s)
{
    bool is_negative = s.value[0] == '-';
    int rv = 0;
    int multiplier = 1;
    size_t limit = is_negative ? 1 : 0;

    for (size_t i = s.length - 1; i > limit; --i) {
        rv += ctoi(s.value[i - 1]) * multiplier;
        multiplier = multiplier == 1 ? 10 : multiplier * 10;
    }

    if (rv > 0 && is_negative)
        rv *= -1;

    return (Num){.type = N_INT, .value.i = rv };
}

enodiscard static inline Str* numtostr(Num n, Arena* restrict arena)
{
    char s[20];
    if (n.type == N_DBL)
        sprintf(s, "%f", n.value.d);
    else
        sprintf(s, "%d", n.value.i);
    return estrnew(s, arena);
}

#define NUMCMP(n1, n2, op)                  \
    if (n1.type != n2.type)                 \
        return false;                       \
                                            \
    if (n1.type == N_DBL)                   \
        return n1.value.d op n2.value.d;    \
                                            \
    return n1.value.i op n2.value.i

enodiscard static inline bool numeq(Num n1, Num n2)
{
    NUMCMP(n1, n2, ==);
}

enodiscard static inline bool numlt(Num n1, Num n2)
{
    NUMCMP(n1, n2, <);
}

enodiscard static inline bool numle(Num n1, Num n2)
{
    NUMCMP(n1, n2, <=);
}

enodiscard static inline bool numgt(Num n1, Num n2)
{
    NUMCMP(n1, n2, >);
}

enodiscard static inline bool numge(Num n1, Num n2)
{
    NUMCMP(n1, n2, >=);
}

enodiscard static inline int numpowi(Num base, Num exp)
{
    int rv = 1;
    while (exp.value.i > 0) {
        rv *= base.value.i;
        exp.value.i--;
    }
    return rv;
}

/*enodiscard static inline Num estrtonum(Str s)
{
    bool is_negative = s.value[0] == '-';
    ssize_t has_commas = estridx(&s, ',');
    ssize_t is_dec = estridx(&s, '.');
    int rv = 0;

    if (is_dec == -1) {
        size_t multiplier = 1;
        for (size_t i = s.length - 1; i > 0; --i) {

        }

        if (rv > 0 && is_negative)
            rv *= -1;

        return (Num){.type = N_INT, .value.i = rv };
    }

    return (Num){.type = N_DBL, .value.d = 0};
}*/
