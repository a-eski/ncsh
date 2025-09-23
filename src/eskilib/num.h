/* Copyright eskilib by Alex Eski 2025 */

#pragma once

#include "str.h"
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
// #include <errno.h>
// #include <math.h>
#include <stdlib.h>

// start with non decimal numbers
// TODO: work on a solution for checking then parsing out decimals
static inline bool isnum(Str s)
{
    if (s.length < 2) {
        puts("invalid len");
        return false;
    }

    for (size_t i = 0; i < s.length - 1; ++i) {
        if (!isdigit(s.value[i])) {
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
