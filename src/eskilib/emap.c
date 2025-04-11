/* Copyright eskilib by Alex Eski 2024 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "emap.h"

bool emap_malloc(struct Arena* const arena, struct emap* table)
{
    table->size = 0;
    table->capacity = ESKILIB_EMAP_DEFAULT_CAPACITY;

    table->entries = arena_malloc(arena, table->capacity, struct emap_Entry);
    if (table->entries == NULL) {
        return false;
    }
    return true;
}

#define ESKILIB_FNV_OFFSET 2166136261
#define ESKILIB_FNV_PRIME 16777619

// 64-bit FNV-1a hash
uint64_t emap_key(const char* str)
{
    register uint64_t i;

    for (i = ESKILIB_FNV_OFFSET; *str; str++) {
        i += (i << 1) + (i << 4) + (i << 7) + (i << 8) + (i << 24);
        i ^= (uint64_t)*str;
    }

    return i;
}

struct estr emap_get(const char* key, struct emap* table)
{
    uint64_t hash = emap_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL) {
        if (key[0] == table->entries[index].key[0] && !strcmp(key, table->entries[index].key)) {
            return table->entries[index].value;
        }

        index++; // linear probing
        if (index >= table->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return estr_Empty;
}

bool emap_exists(const char* key, struct emap* table)
{
    uint64_t hash = emap_key(key);
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

const char* emap_set_entry(struct emap_Entry* entries, size_t capacity, const char* key,
                                        struct estr value, size_t* plength)
{
    uint64_t hash = emap_key(key);
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

bool emap_expand(struct Arena* const arena, struct emap* table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;
    }

    struct emap_Entry* new_entries = arena_malloc(arena, new_capacity, struct emap_Entry);
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        struct emap_Entry entry = table->entries[i];
        if (entry.key != NULL) {
            emap_set_entry(new_entries, new_capacity, entry.key, entry.value, NULL);
        }
    }

    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char* emap_set(const char* key, struct estr value, struct Arena* const arena,
                                  struct emap* table)
{
    assert(value.value != NULL && value.length > 0);
    if (value.value == NULL || value.length == 0) {
        return NULL;
    }

    if (table->size >= table->capacity / 2) {
        if (!emap_expand(arena, table)) {
            return NULL;
        }
    }

    return emap_set_entry(table->entries, table->capacity, key, value, &table->size);
}
