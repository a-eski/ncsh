/* Copyright ncsh (C) by Alex Eski 2024 */
/* hashset.h: Simple hashset implementation */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"

#define HASHSET_DEFAULT_CAPACITY 100

typedef struct {
    char* key;
    Str value;
} Hashset_Entry;

typedef struct {
    size_t size;
    size_t capacity;
    Hashset_Entry* entries;
} Hashset;

void hashset_malloc(size_t capacity, Arena* restrict arena, Hashset* restrict hset);

Str hashset_get(char* restrict key, Hashset* restrict hset);

bool hashset_exists(char* restrict key, Hashset* restrict hset);

char* hashset_set(Str val, Arena* restrict arena, Hashset* restrict hset);
