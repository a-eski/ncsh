/* Copyright ncsh (C) by Alex Eski 2024 */
/* variables.c: a basic hashmap using liner probing */

#pragma once

#include <stddef.h>

#include "arena.h"
#include "eskilib/estr.h"

#define VARS_DEFAULT_CAPACITY 100

struct Vars_Entry {
    const char* key;
    struct estr value;
};

struct Vars {
    size_t size;
    size_t capacity;
    struct Vars_Entry* entries;
};

void vars_malloc(struct Arena* const arena, struct Vars* restrict vars);

struct estr* vars_get(char* key, struct Vars* restrict vars);

bool vars_exists(char* key, struct Vars* restrict vars);

const char* vars_set(char* key, struct estr* val, struct Arena* const arena, struct Vars* restrict vars);
