/* Copyright ncsh by Alex Eski 2025 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../defines.h"
#include "../eskilib/estr.h"
#include "autocompletions.h"

static inline int char_to_index(const char character);
static inline char index_to_char(const int index);

struct Autocompletion_Node* ac_alloc(struct Arena* const arena)
{
    struct Autocompletion_Node* tree = arena_malloc(arena, 1, struct Autocompletion_Node);
    tree->is_end_of_a_word = false;
    return tree;
}

void ac_add(const char* const string, const size_t length, struct Autocompletion_Node* restrict tree,
            struct Arena* const arena)
{
    assert(string && length && tree && arena);
    if (!string || !length || length > NCSH_MAX_INPUT) {
        return;
    }

    for (size_t i = 0; i < length - 1; ++i) { // string.length - 1 because it includes null terminator
        int index = char_to_index(string[i]);
        if (index < 0 || index > 96) {
            continue;
        }

        if (!tree->nodes[index]) {
            tree->nodes[index] = arena_malloc(arena, 1, struct Autocompletion_Node);
            tree->nodes[index]->is_end_of_a_word = false;
            tree->nodes[index]->weight = 1;
            tree = tree->nodes[index];
            continue;
        }

        ++tree->nodes[index]->weight;
        tree = tree->nodes[index];
    }

    tree->is_end_of_a_word = true;
}

void ac_add_multiple(const struct estr* const strings, const int count, struct Autocompletion_Node* restrict tree,
                     struct Arena* const arena)
{
    assert(strings && tree && arena);
    if (count <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        ac_add(strings[i].value, strings[i].length, tree, arena);
    }
}

struct Autocompletion_Node* ac_find(const char* prefix, struct Autocompletion_Node* restrict tree)
{
    if (!prefix || !*prefix) {
        return tree;
    }

    if (!tree) {
        return NULL;
    }

    return ac_find(prefix + 1, tree->nodes[char_to_index(*prefix)]);
}

static uint_fast32_t string_position;
static uint_fast8_t matches_position;

void ac_match(struct Autocompletion* const matches, struct Autocompletion_Node* restrict tree,
              struct Arena* const restrict scratch_arena)
{
    for (int i = 0; i < NCSH_LETTERS; ++i) {
        if (!tree->nodes[i]) {
            continue;
        }

        if (matches_position + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
            return;
        }

        if (!matches[matches_position].value) {
            matches[matches_position].value = arena_malloc(scratch_arena, NCSH_MAX_INPUT, char);

            if (matches_position) {
                memcpy(matches[matches_position].value, matches[matches_position - 1].value, string_position);
            }
        }

        matches[matches_position].value[string_position] = index_to_char(i);
        ++string_position;

        if (tree->nodes[i]->is_end_of_a_word) {
            matches[matches_position].weight = tree->nodes[i]->weight;
            ++matches_position;
        }

        ac_match(matches, tree->nodes[i], scratch_arena);

        if (matches_position + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
            return;
        }

        if (matches[matches_position].value) {
            ++matches_position;
        }

        --string_position;
    }
}

uint_fast8_t ac_matches(struct Autocompletion* const matches, struct Autocompletion_Node* restrict tree,
                        struct Arena* const restrict scratch_arena)
{
    string_position = 0;
    matches_position = 0;

    ac_match(matches, tree, scratch_arena);

    return matches_position;
}

uint_fast8_t ac_get(const char* const search, struct Autocompletion* matches, struct Autocompletion_Node* restrict tree,
                    struct Arena scratch_arena)
{
    assert(search);
    struct Autocompletion_Node* const prefix = ac_find(search, tree);
    if (!prefix) {
        return 0;
    }

    uint_fast8_t match_count = ac_matches(matches, prefix, &scratch_arena);
    if (!match_count) {
        return 0;
    }

    return match_count;
}

uint_fast8_t ac_first(const char* const search, char* match, struct Autocompletion_Node* restrict tree,
                      struct Arena scratch_arena)
{
    assert(search);
    struct Autocompletion_Node* prefix = ac_find(search, tree);
    if (!prefix) {
        return 0;
    }

    struct Autocompletion matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast8_t match_count = ac_matches(matches, prefix, &scratch_arena);
    if (!match_count) {
        return 0;
    }

    struct Autocompletion potential_match = matches[0];
    for (uint_fast8_t i = 1; i < match_count; ++i) {
        if (matches[i].weight > potential_match.weight) {
            potential_match = matches[i];
        }
    }

    memcpy(match, potential_match.value, NCSH_MAX_INPUT);

    return 1;
}
