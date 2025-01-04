#ifndef eskilib_hashtable_h
#define eskilib_hashtable_h

#include <stdbool.h>
#include <stddef.h>

#include "eskilib_string.h"

#ifndef ESKILIB_HASHTABLE_DEFAULT_CAPACITY
#define ESKILIB_HASHTABLE_DEFAULT_CAPACITY 100
#endif // !ESKILIB_HASHTABLE_DEFAULT_CAPACITY

struct eskilib_HashTable_Entry {
    const char* key;
    struct eskilib_String value;
};

struct eskilib_HashTable {
    size_t capacity;
    size_t length;
    struct eskilib_HashTable_Entry* entries;
};

bool eskilib_hashtable_malloc(struct eskilib_HashTable* table);

void eskilib_hashtable_free(struct eskilib_HashTable* table);

struct eskilib_String eskilib_hashtable_get(const char* key, struct eskilib_HashTable* table);

const char* eskilib_hashtable_set(const char* key, struct eskilib_String value, struct eskilib_HashTable* table);

#endif // eskilib_hashtable_h

