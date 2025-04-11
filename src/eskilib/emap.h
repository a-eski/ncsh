/* Copyright eskilib by Alex Eski 2024 */

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

bool emap_malloc(struct Arena* const arena, struct emap* table);

struct estr emap_get(const char* key, struct emap* table);

bool emap_exists(const char* key, struct emap* table);

const char* emap_set(const char* key, struct estr value, struct Arena* const arena,
                                  struct emap* table);
