/* Copyright ncsh (C) by Alex Eski 2024 */
/* variables.c: a basic hashmap using liner probing */

#pragma once

#include <stddef.h>

#include "../../arena.h"
#include "../../eskilib/str.h"

#define VARS_DEFAULT_CAPACITY 100

typedef struct {
    char* key;
    Str value;
} Vars_Entry;

typedef struct {
    size_t size;
    size_t capacity;
    Vars_Entry* entries;
} Vars;

void vars_malloc(Arena* rst arena, Vars* rst vars);

Str* vars_get(char* rst key, Vars* rst vars);

bool vars_exists(char* rst key, Vars* rst vars);

char* vars_set(char* rst key, Str* rst val, Arena* rst arena, Vars* rst vars);
