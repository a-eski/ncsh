/* Copyright ncsh (C) by Alex Eski 2024 */
/* hashset.h: Simple hashset implementation */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"

#define HASHSET_DEFAULT_CAPACITY 100

struct Hashset_Entry {
    char* key;
    struct Str value;
};

struct Hashset {
    size_t size;
    size_t capacity;
    struct Hashset_Entry* entries;
};

void hashset_malloc(size_t capacity, struct Arena* restrict arena, struct Hashset* restrict hset);

struct Str hashset_get(char* restrict key, struct Hashset* restrict hset);

bool hashset_exists(char* restrict key, struct Hashset* restrict hset);

char* hashset_set(struct Str val, struct Arena* restrict arena, struct Hashset* restrict hset);
