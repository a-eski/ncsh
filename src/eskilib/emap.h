/* Copyright eskilib by Alex Eski 2024 */
/* emap: Optimized for keys having the same value as values */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "estr.h"

#define ESKILIB_EMAP_DEFAULT_CAPACITY 100

struct emap_Entry {
    char* key;
    struct estr value;
};

struct emap {
    size_t size;
    size_t capacity;
    struct emap_Entry* entries;
};

void emap_malloc(struct Arena* restrict arena, struct emap* restrict hmap);

struct estr emap_get(char* restrict key, struct emap* restrict hmap);

bool emap_exists(char* restrict key, struct emap* restrict hmap);

char* emap_set(struct estr val, struct Arena* restrict arena, struct emap* restrict hmap);
