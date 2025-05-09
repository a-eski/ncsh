/* Copyright ncsh (C) by Alex Eski 2024 */
/* hashset.c: Simple hashset implementation */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "hashset.h"

void hashset_malloc(size_t capacity, struct Arena* restrict arena, struct Hashset* restrict hset)
{
    hset->size = 0;
    hset->capacity = capacity == 0 ? HASHSET_DEFAULT_CAPACITY : capacity;

    hset->entries = arena_malloc(arena, hset->capacity, struct Hashset_Entry);
}

#define ESKILIB_FNV_OFFSET 2166136261
#define ESKILIB_FNV_PRIME 16777619

// 64-bit FNV-1a hash
uint64_t hashset_key(char* restrict str)
{
    register uint64_t i;

    for (i = ESKILIB_FNV_OFFSET; *str; str++) {
        i += (i << 1) + (i << 4) + (i << 7) + (i << 8) + (i << 24);
        i ^= (uint64_t)*str;
    }

    return i;
}

struct Str hashset_get(char* restrict key, struct Hashset* restrict hset)
{
    uint64_t hash = hashset_key(key);
    size_t index = (size_t)(hash & (uint64_t)(hset->capacity - 1));

    while (hset->entries[index].key) {
        if (key[0] == hset->entries[index].key[0] && !strcmp(key, hset->entries[index].key)) {
            return hset->entries[index].value;
        }

        ++index; // linear probing
        if (index >= hset->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return Str_Empty;
}

bool hashset_exists(char* restrict key, struct Hashset* restrict hset)
{
    uint64_t hash = hashset_key(key);
    size_t index = (size_t)(hash & (uint64_t)(hset->capacity - 1));

    while (hset->entries[index].key) {
        if (key[0] == hset->entries[index].key[0] && !strcmp(key, hset->entries[index].key)) {
            return true;
        }

        ++index; // linear probing
        if (index >= hset->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return false;
}

char* hashset_set_entry(struct Hashset_Entry* restrict entries, size_t capacity, struct Str val, size_t* plength)
{
    uint64_t hash = hashset_key(val.value);
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

bool hashset_expand(struct Arena* restrict arena, struct Hashset* restrict hset)
{
    size_t new_capacity = hset->capacity * 2;
    if (new_capacity < hset->capacity) {
        return false;
    }

    struct Hashset_Entry* new_entries = arena_malloc(arena, new_capacity, struct Hashset_Entry);

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < hset->capacity; i++) {
        struct Hashset_Entry entry = hset->entries[i];
        if (entry.key != NULL) {
            hashset_set_entry(new_entries, new_capacity, entry.value, NULL);
        }
    }

    hset->entries = new_entries;
    hset->capacity = new_capacity;
    return true;
}

char* hashset_set(struct Str val, struct Arena* restrict arena, struct Hashset* restrict hset)
{
    assert(val.value && val.length);
    if (!val.value || !val.length) {
        return NULL;
    }

    if (hset->size >= hset->capacity / 2) {
        if (!hashset_expand(arena, hset)) {
            return NULL;
        }
    }

    return hashset_set_entry(hset->entries, hset->capacity, val, &hset->size);
}
