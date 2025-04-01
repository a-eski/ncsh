/* Copyright eskilib by Alex Eski 2024 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "eskilib_hashtable.h"

bool eskilib_hashtable_malloc(struct ncsh_Arena* const arena, struct eskilib_HashTable* table)
{
    table->size = 0;
    table->capacity = ESKILIB_HASHTABLE_DEFAULT_CAPACITY;

    table->entries = arena_malloc(arena, table->capacity, struct eskilib_HashTable_Entry);
    if (table->entries == NULL) {
        return false;
    }
    return true;
}

#define ESKILIB_FNV_OFFSET 14695981039346656037UL
#define ESKILIB_FNV_PRIME 1099511628211UL

// 64-bit FNV-1a hash
uint64_t eskilib_hashtable_key(const char* key)
{
    uint64_t hash = ESKILIB_FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= ESKILIB_FNV_PRIME;
    }

    return hash;
}

struct eskilib_String eskilib_hashtable_get(const char* key, struct eskilib_HashTable* table)
{
    uint64_t hash = eskilib_hashtable_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL) {
        if (strcmp(key, table->entries[index].key) == 0) {
            return table->entries[index].value;
        }

        index++; // linear probing
        if (index >= table->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return eskilib_String_Empty;
}

bool eskilib_hashtable_exists(const char* key, struct eskilib_HashTable* table)
{
    uint64_t hash = eskilib_hashtable_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL) {
        if (strcmp(key, table->entries[index].key) == 0) {
            return true;
        }

        index++; // linear probing
        if (index >= table->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return false;
}

const char* eskilib_hashtable_set_entry(struct eskilib_HashTable_Entry* entries, size_t capacity, const char* key,
                                        struct eskilib_String value, size_t* plength)
{
    uint64_t hash = eskilib_hashtable_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }

        index++; // linear probing
        if (index >= capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    if (plength != NULL) {
        /*key = strdup(key);
        if (key == NULL) {
            return NULL;
        }*/
        (*plength)++;
    }

    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}

bool eskilib_hashtable_expand(struct ncsh_Arena* const arena, struct eskilib_HashTable* table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;
    }

    struct eskilib_HashTable_Entry* new_entries = arena_malloc(arena, new_capacity, struct eskilib_HashTable_Entry);
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        struct eskilib_HashTable_Entry entry = table->entries[i];
        if (entry.key != NULL) {
            eskilib_hashtable_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);
        }
    }

    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char* eskilib_hashtable_set(const char* key,
                                  struct eskilib_String value,
                                  struct ncsh_Arena* const arena,
                                  struct eskilib_HashTable* table)
{
    assert(value.value != NULL && value.length > 0);
    if (value.value == NULL || value.length == 0) {
        return NULL;
    }

    if (table->size >= table->capacity / 2) {
        if (!eskilib_hashtable_expand(arena, table)) {
            return NULL;
        }
    }

    return eskilib_hashtable_set_entry(table->entries, table->capacity, key, value, &table->size);
}
