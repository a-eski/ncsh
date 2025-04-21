/* Copyright ncsh (C) by Alex Eski 2024 */
/* variables.c: a basic hashmap using liner probing */

#pragma once

#include <stddef.h>

#include "arena.h"
#include "eskilib/estr.h"

#define VAR_DEFAULT_CAPACITY 100

struct var_Entry {
    const char* key;
    struct estr value;
};

struct var {
    size_t size;
    size_t capacity;
    struct var_Entry* entries;
};

void var_malloc(struct Arena* const arena, struct var* restrict vars);

struct estr* var_get(char* key, struct var* restrict vars);

bool var_exists(char* key, struct var* restrict vars);

const char* var_set(char* key, struct estr* val, struct Arena* const arena, struct var* restrict vars);
