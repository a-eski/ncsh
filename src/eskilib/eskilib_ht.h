#ifndef eskilib_ht_h
#define eskilib_ht_h

#include <stdbool.h>
#include <stddef.h>

struct eskilib_ht_entry {
    const char* key;
    char* value;
};

struct eskilib_ht {
    size_t capacity;
    size_t length;
    struct eskilib_ht_entry* entries;
};

bool eskilib_ht_malloc(struct eskilib_ht* table);

void eskilib_ht_free(struct eskilib_ht* table);

char* eskilib_ht_get(const char* key, struct eskilib_ht* table);

const char* eskilib_ht_set(const char* key, char* value, struct eskilib_ht* table);

#endif // eskilib_ht_h

