#include "eskilib_ht.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ESKILIB_HT_DEFAULT_CAPACITY 100

bool eskilib_ht_malloc(struct eskilib_ht* table) {
    table->length = 0;
    table->capacity = ESKILIB_HT_DEFAULT_CAPACITY;

    table->entries = calloc(table->capacity, sizeof(struct eskilib_ht_entry));
    if (table->entries == NULL) {
        return false;
    }
    return true;
}

void eskilib_ht_free(struct eskilib_ht* table) {
    for (size_t i = 0; i < table->capacity; i++) {
        free((char*)table->entries[i].key);
    }

    free(table->entries);
}

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// 64-bit FNV-1a hash
uint64_t hash_key(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

char* eskilib_ht_get(const char* key, struct eskilib_ht* table) {
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL) {
        if (strcmp(key, table->entries[index].key) == 0) {
            return table->entries[index].value;
        }

        index++; //linear probing
        if (index >= table->capacity) {
            index = 0; //at end of ht, wrap around
        }
    }

    return NULL;
}

const char* eskilib_ht_set_entry(struct eskilib_ht_entry* entries, size_t capacity, const char* key, char* value, size_t* plength) {
    uint64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }

        index++; //linear probing
        if (index >= capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }

    entries[index].key = (char*)key;
    entries[index].value = value;
    return key;
}

bool eskilib_ht_expand(struct eskilib_ht* table) {
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) {
        return false;
    }

    struct eskilib_ht_entry* new_entries = calloc(new_capacity, sizeof(struct eskilib_ht_entry));
    if (new_entries == NULL) {
        return false;
    }

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->capacity; i++) {
        struct eskilib_ht_entry entry = table->entries[i];
        if (entry.key != NULL) {
            eskilib_ht_set_entry(new_entries, new_capacity, entry.key,
                         entry.value, NULL);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_capacity;
    return true;
}

const char* eskilib_ht_set(const char* key, char* value, struct eskilib_ht* table) {
    assert(value != NULL);
    if (value == NULL) {
        return NULL;
    }

    if (table->length >= table->capacity / 2) {
        if (!eskilib_ht_expand(table)) {
            return NULL;
        }
    }

    return eskilib_ht_set_entry(table->entries, table->capacity, key, value,
                        &table->length);
}

