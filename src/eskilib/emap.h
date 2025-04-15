/* Copyright eskilib by Alex Eski 2024 */
/* emap: Optimized for keys having the same value as values */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "../arena.h"
#include "estr.h"

#define ESKILIB_EMAP_DEFAULT_CAPACITY 100

struct emap_Entry {
    const char* key;
    struct estr value;
};

struct emap {
    size_t size;
    size_t capacity;
    struct emap_Entry* entries;
};

void emap_malloc(struct Arena* const arena, struct emap* hmap);

struct estr emap_get(char* key, struct emap* hmap);

bool emap_exists(char* key, struct emap* hmap);

const char* emap_set(struct estr val, struct Arena* const arena, struct emap* hmap);
