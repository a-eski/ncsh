/* Copyright eskilib by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "eskilib_string.h"
#include "../ncsh_arena.h"

#define HMAP_DEFAULT_CAPACITY 100

struct hmap_Entry {
    const char* key;
    struct eskilib_String value;
};

struct hmap {
    size_t size;
    size_t capacity;
    struct eskilib_hmap_Entry* entries;
};

bool hmap_malloc(struct ncsh_Arena* const arena,
                              struct hmap* table);

struct eskilib_String hmap_get(const char* key,
                                            struct hmap* table);

bool hmap_exists(const char* key,
                              struct hmap* table);

const char* hmap_set(const char* key,
                                  struct eskilib_String value,
                                  struct ncsh_Arena* const arena,
                                  struct hmap* table);
