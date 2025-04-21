/* Copyright ncsh by Alex Eski 2025 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../defines.h" // used for NCSH_MAX_INPUT macro
#include "../eskilib/estr.h"
#include "ac.h"

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

/* ac_find
 * const char* p: the prefix, struct Autocompletion_Node* restrict t: the trie
 * Walk the trie to find the prefix. Return null if prefix not found.
 */
struct Autocompletion_Node* ac_find(const char* p, struct Autocompletion_Node* restrict t)
{
    while (p && *p) {
        if (!t)
            return NULL;

        t = t->nodes[char_to_index(*p)];
        ++p;
    }

    return t;
}

// static slightly improved performance in benchmarks
static uint_fast32_t ac_str_pos;
static uint_fast8_t ac_match_pos;

// not using static slightly improved performance in benchmarks
char ac_buffer[NCSH_MAX_INPUT];
size_t ac_buffer_len;

void ac_match(struct Autocompletion* const matches, struct Autocompletion_Node* restrict tree,
              struct Arena* const restrict scratch)
{
    if (!tree || ac_match_pos + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
        return;
    }

    if (tree->is_end_of_a_word && *ac_buffer) {
        matches[ac_match_pos].value = arena_malloc(scratch, ac_buffer_len + 1, char);
        memcpy(matches[ac_match_pos].value, ac_buffer, ac_buffer_len);
        matches[ac_match_pos].value[ac_buffer_len] = '\0';
        matches[ac_match_pos].weight = tree->weight;
        ++ac_match_pos;
    }

    for (size_t i = 0; i < NCSH_LETTERS; ++i) {
        if (!tree->nodes[i]) {
            continue;
        }

        ac_buffer[ac_buffer_len] = index_to_char(i);
        ++ac_buffer_len;

        ac_match(matches, tree->nodes[i], scratch);

        if (ac_match_pos + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
            return;
        }

        if (matches[ac_match_pos].value) {
            ++ac_match_pos;
        }

        --ac_buffer_len;
    }
}

uint_fast8_t ac_matches(struct Autocompletion* const matches, struct Autocompletion_Node* prefix,
                        struct Arena* const restrict scratch)
{
    ac_str_pos = 0;
    ac_match_pos = 0;
    ac_buffer[0] = '\0';
    ac_buffer_len = 0;

    ac_match(matches, prefix, scratch);

    return ac_match_pos;
}

uint_fast8_t ac_get(const char* search, struct Autocompletion* matches, struct Autocompletion_Node* restrict tree,
                    struct Arena scratch)
{
    assert(search);

    struct Autocompletion_Node* prefix = ac_find(search, tree);
    if (!prefix)
        return 0;

    uint_fast8_t match_count = ac_matches(matches, prefix, &scratch);
    if (!match_count)
        return 0;

    return match_count;
}

uint_fast8_t ac_first(const char* search, char* match, struct Autocompletion_Node* restrict tree, struct Arena scratch)
{
    assert(search);

    struct Autocompletion_Node* prefix = ac_find(search, tree);
    if (!prefix)
        return 0;

    struct Autocompletion matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast8_t match_count = ac_matches(matches, prefix, &scratch);
    if (!match_count)
        return 0;

    struct Autocompletion potential_match = matches[0];
    for (uint_fast8_t i = 1; i < match_count; ++i) {
        if (matches[i].weight > potential_match.weight) {
            potential_match = matches[i];
        }
    }

    memcpy(match, potential_match.value, NCSH_MAX_INPUT);

    return 1;
}
