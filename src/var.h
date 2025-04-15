/* Copyright ncsh (C) by Alex Eski 2024 */
/* variables.c: a regular hashmap */

#pragma once

#include <stdbool.h>
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

void var_malloc(struct Arena* const arena, struct var* hmap);

struct estr* var_get(char* key, struct var* hmap);

bool var_exists(char* key, struct var* hmap);

const char* var_set(char* key, struct estr* val, struct Arena* const arena, struct var* hmap);
