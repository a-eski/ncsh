/* Copyright ncsh (C) by Alex Eski 2024 */
/* hashset.h: Simple hashset implementation */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"

#define HASHSET_DEFAULT_CAPACITY 100

struct Hashset {
    size_t size;
    size_t capacity;
    uint64_t* entries;
};

void hashset_malloc(struct Arena* restrict arena, struct Hashset* restrict hset);

bool hashset_exists(char* restrict key, struct Hashset* restrict hset);

char* hashset_set(struct Str val, struct Arena* restrict arena, struct Hashset* restrict hset);
