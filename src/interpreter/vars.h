/* Copyright ncsh (C) by Alex Eski 2024 */
/* variables.c: a basic hashmap using liner probing */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"

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

void vars_malloc(Arena* restrict arena, Vars* restrict vars);

Str* vars_get(char* restrict key, Vars* restrict vars);

bool vars_exists(char* restrict key, Vars* restrict vars);

char* vars_set(char* restrict key, Str* restrict val, Arena* restrict arena, Vars* restrict vars);
