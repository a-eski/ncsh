/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "eskilib_string.h"

#define ESKILIB_HASHTABLE_DEFAULT_CAPACITY 100

struct eskilib_HashTable_Entry {
    const char* key;
    struct eskilib_String value;
};

struct eskilib_HashTable {
    size_t size;
    size_t capacity;
    struct eskilib_HashTable_Entry* entries;
};

bool eskilib_hashtable_malloc(struct eskilib_HashTable* table);

void eskilib_hashtable_free(struct eskilib_HashTable* table);

struct eskilib_String eskilib_hashtable_get(const char* key, struct eskilib_HashTable* table);

bool eskilib_hashtable_exists(const char* key, struct eskilib_HashTable* table);

const char* eskilib_hashtable_set(const char* key, struct eskilib_String value, struct eskilib_HashTable* table);
