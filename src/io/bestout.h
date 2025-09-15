#pragma once

#include <stddef.h>
#include <stdio.h>

#include "../eskilib/str.h"

#define AUTOCOMPLETE_YELLOW 220
#define AUTOCOMPLETE_DIM 244

int bestlineWriteChars(int fd, const char *p);
int bestlineWrite(int fd, const void *p, size_t n);

static inline int bestlineWriteStr(int fd, Str str) {
    return bestlineWrite(fd, str.value, str.length - 1);
}
