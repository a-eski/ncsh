/* Copyright eskilib by Alex Eski 2024 */
/* emap: Optimized for keys having the same value as values */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "emap.h"

void emap_malloc(struct Arena* const arena, struct emap* hmap)
{
    hmap->size = 0;
    hmap->capacity = ESKILIB_EMAP_DEFAULT_CAPACITY;

    hmap->entries = arena_malloc(arena, hmap->capacity, struct emap_Entry);
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

struct estr emap_get(char* key, struct emap* hmap)
{
    uint64_t hash = emap_key(key);
    size_t index = (size_t)(hash & (uint64_t)(hmap->capacity - 1));

    while (hmap->entries[index].key) {
        if (key[0] == hmap->entries[index].key[0] && !strcmp(key, hmap->entries[index].key)) {
            return hmap->entries[index].value;
        }

        ++index; // linear probing
        if (index >= hmap->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return estr_Empty;
}

bool emap_exists(char* key, struct emap* hmap)
{
    uint64_t hash = emap_key(key);
    size_t index = (size_t)(hash & (uint64_t)(hmap->capacity - 1));

    while (hmap->entries[index].key) {
        if (key[0] == hmap->entries[index].key[0] && !strcmp(key, hmap->entries[index].key)) {
            return true;
        }

        ++index; // linear probing
        if (index >= hmap->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return false;
}

const char* emap_set_entry(struct emap_Entry* entries, size_t capacity, struct estr val, size_t* plength)
{
    uint64_t hash = emap_key(val.value);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key) {
        if (!strcmp(val.value, entries[index].key)) {
            entries[index].value = val;
            return entries[index].key;
        }

        ++index; // linear probing
        if (index >= capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    if (plength) {
        (*plength)++;
    }

    entries[index].key = val.value;
    entries[index].value = val;
    return val.value;
}

bool emap_expand(struct Arena* const arena, struct emap* hmap)
{
    size_t new_capacity = hmap->capacity * 2;
    if (new_capacity < hmap->capacity) {
        return false;
    }

    struct emap_Entry* new_entries = arena_malloc(arena, new_capacity, struct emap_Entry);

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < hmap->capacity; i++) {
        struct emap_Entry entry = hmap->entries[i];
        if (entry.key != NULL) {
            emap_set_entry(new_entries, new_capacity, entry.value, NULL);
        }
    }

    hmap->entries = new_entries;
    hmap->capacity = new_capacity;
    return true;
}

const char* emap_set(struct estr val, struct Arena* const arena, struct emap* hmap)
{
    assert(val.value && val.length);
    if (!val.value || !val.length) {
        return NULL;
    }

    if (hmap->size >= hmap->capacity / 2) {
        if (!emap_expand(arena, hmap)) {
            return NULL;
        }
    }

    return emap_set_entry(hmap->entries, hmap->capacity, val, &hmap->size);
}
