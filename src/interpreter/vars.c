/* Copyright eskilib by Alex Eski 2024 */
/* var: a simple hasvars that can be optimized for storing variables */
/* currently very simple implementation using FNV-1a hash and linear probing */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../arena.h"
#include "vars.h"

void vars_malloc(Arena* restrict arena, Vars* restrict vars)
{
    vars->size = 0;
    vars->capacity = VARS_DEFAULT_CAPACITY;

    vars->entries = arena_malloc(arena, vars->capacity, Vars_Entry);
}

#define VARS_FNV_OFFSET 2166136261

// 64-bit FNV-1a hash
[[nodiscard]]
uint64_t vars_key(char* restrict str)
{
    register uint64_t i;

    for (i = VARS_FNV_OFFSET; *str; str++) {
        i += (i << 1) + (i << 4) + (i << 7) + (i << 8) + (i << 24);
        i ^= (uint64_t)*str;
    }

    return i;
}

[[nodiscard]]
Str* vars_get(char* restrict key, Vars* restrict vars)
{
    uint64_t hash = vars_key(key);
    size_t index = (size_t)(hash & (uint64_t)(vars->capacity - 1));

    while (vars->entries[index].key) {
        if (key[0] == vars->entries[index].key[0] && !strcmp(key, vars->entries[index].key)) {
            return &vars->entries[index].value;
        }

        ++index; // linear probing
        if (index >= vars->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return NULL;
}

[[nodiscard]]
bool vars_exists(char* restrict key, Vars* restrict vars)
{
    uint64_t hash = vars_key(key);
    size_t index = (size_t)(hash & (uint64_t)(vars->capacity - 1));

    while (vars->entries[index].key) {
        if (key[0] == vars->entries[index].key[0] && !strcmp(key, vars->entries[index].key)) {
            return true;
        }

        ++index; // linear probing
        if (index >= vars->capacity) {
            index = 0; // at end of ht, wrap around
        }
    }

    return false;
}

[[nodiscard]]
char* vars_set_entry(Vars_Entry* restrict entries, size_t capacity, char* restrict key, Str* restrict val, size_t* restrict plength)
{
    uint64_t hash = vars_key(key);
    size_t index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (entries[index].key) {
        if (!strcmp(key, entries[index].key)) {
            entries[index].value = *val;
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

    entries[index].key = key;
    entries[index].value = *val;
    return key;
}

[[nodiscard]]
bool vars_expand(char* restrict key, Arena* restrict arena, Vars* restrict vars)
{
    size_t new_capacity = vars->capacity * 2;
    if (new_capacity < vars->capacity) {
        return false;
    }

    Vars_Entry* new_entries = arena_malloc(arena, new_capacity, Vars_Entry);

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < vars->capacity; i++) {
        Vars_Entry entry = vars->entries[i];
        if (entry.key != NULL) {
            if (!vars_set_entry(new_entries, new_capacity, key, &entry.value, NULL))
                return false;
        }
    }

    vars->entries = new_entries;
    vars->capacity = new_capacity;
    return true;
}

[[nodiscard]]
char* vars_set(char* restrict key, Str* restrict val, Arena* restrict arena, Vars* restrict vars)
{
    assert(val->value && val->length);
    if (!val->value || !val->length) {
        return NULL;
    }

    if (vars->size >= vars->capacity / 2) {
        if (!vars_expand(key, arena, vars)) {
            return NULL;
        }
    }

    return vars_set_entry(vars->entries, vars->capacity, key, val, &vars->size);
}
