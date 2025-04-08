/* Copyright ncsh by Alex Eski 2025 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../eskilib/eskilib_string.h"
#include "../ncsh_defines.h"
#include "ncsh_autocompletions.h"

int ncsh_char_to_index(char character);
char ncsh_index_to_char(int index);

struct ncsh_Autocompletion_Node* ncsh_autocompletions_alloc(struct ncsh_Arena* const arena)
{
    struct ncsh_Autocompletion_Node* tree = arena_malloc(arena, 1, struct ncsh_Autocompletion_Node);
    tree->nodes = arena_malloc(arena, NCSH_LETTERS, struct ncsh_Autocompletion_Node*);
    tree->is_end_of_a_word = false;
    return tree;
}

void ncsh_autocompletions_add(const char* const string,
                              const size_t length,
                              struct ncsh_Autocompletion_Node* restrict tree,
                              struct ncsh_Arena* const arena)
{
    assert(string);
    assert(length > 0);
    assert(tree);
    assert(tree->nodes);
    if (!string || !length || !tree || length > NCSH_MAX_INPUT) {
        return;
    }

    for (size_t i = 0; i < length - 1; ++i) { // string.length - 1 because it includes null terminator
        int index = ncsh_char_to_index(string[i]);
        if (index < 0 || index > 96) {
            continue;
        }

        if (!tree->nodes[index]) {
            tree->nodes[index] = arena_malloc(arena, 1, struct ncsh_Autocompletion_Node);
            tree->nodes[index]->nodes = arena_malloc(arena, NCSH_LETTERS, struct ncsh_Autocompletion_Node*);
            tree->nodes[index]->is_end_of_a_word = false;
            tree->nodes[index]->weight = 1;
        }
        else {
            ++tree->nodes[index]->weight;
        }

        tree = tree->nodes[index];
    }

    tree->is_end_of_a_word = true;
}

void ncsh_autocompletions_add_multiple(const struct eskilib_String* const strings,
                                       const int count,
                                       struct ncsh_Autocompletion_Node* restrict tree,
                                       struct ncsh_Arena* const arena)
{
    if (!strings || count <= 0 || !tree) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        ncsh_autocompletions_add(strings[i].value, strings[i].length, tree, arena);
    }
}

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search(const char* const string,
                                                             const size_t length,
                                                             struct ncsh_Autocompletion_Node* restrict tree)
{
    assert(string);
    assert(length > 0);
    assert(tree);
    if (!string || !length || !tree) {
        return NULL;
    }

    for (size_t i = 0; i < length - 1; ++i) {
        int index = ncsh_char_to_index(string[i]);

        if (!tree->nodes[index]) {
            return NULL;
        }

        tree = tree->nodes[index];
    }

    return tree;
}

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search_string(const struct eskilib_String string,
                                                                    struct ncsh_Autocompletion_Node* restrict tree)
{
    return ncsh_autocompletions_search(string.value, string.length, tree);
}

void ncsh_autocompletions_match(struct ncsh_Autocompletion* const matches,
                                uint_fast32_t* const string_position,
                                uint_fast8_t* const matches_position,
                                struct ncsh_Autocompletion_Node* restrict tree,
                                struct ncsh_Arena* const scratch_arena)
{
    for (int i = 0; i < NCSH_LETTERS; ++i) {
        if (tree->nodes[i]) {
            if (*matches_position + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
                return;
            }

            if (!matches[*matches_position].value) {
                matches[*matches_position].value = arena_malloc(scratch_arena, NCSH_MAX_INPUT, char);

                if (*string_position > 0 && *matches_position > 0) {
                    memcpy(matches[*matches_position].value, matches[*matches_position - 1].value, *string_position);
                }
            }

            matches[*matches_position].value[*string_position] = ncsh_index_to_char(i);
            ++*string_position;
            matches[*matches_position].value[*string_position] = '\0';

            if (tree->nodes[i]->is_end_of_a_word) {
                matches[*matches_position].weight = tree->nodes[i]->weight;
                ++*matches_position;
            }

            ncsh_autocompletions_match(matches, string_position, matches_position, tree->nodes[i], scratch_arena);

            if (matches[*matches_position].value) {
                if (*matches_position + 1 < NCSH_MAX_AUTOCOMPLETION_MATCHES) {
                    ++*matches_position;
                }
                else {
                    return;
                }
            }

            *string_position = *string_position - 1;
        }
    }
}

uint_fast8_t ncsh_autocompletions_matches(struct ncsh_Autocompletion* const matches,
                                          struct ncsh_Autocompletion_Node* restrict tree,
                                          struct ncsh_Arena* const scratch_arena)
{
    uint_fast32_t string_position = 0;
    uint_fast8_t matches_position = 0;

    ncsh_autocompletions_match(matches, &string_position, &matches_position, tree, scratch_arena);

    return matches_position;
}

uint_fast8_t ncsh_autocompletions_get(const char* const search,
                                      const size_t search_length,
                                      struct ncsh_Autocompletion* matches,
                                      struct ncsh_Autocompletion_Node* restrict tree,
                                      struct ncsh_Arena scratch_arena)
{
    struct ncsh_Autocompletion_Node* const search_result = ncsh_autocompletions_search(search, search_length, tree);
    if (!search_result) {
        return 0;
    }

    uint_fast8_t match_count = ncsh_autocompletions_matches(matches, search_result, &scratch_arena);
    if (!match_count) {
        return 0;
    }

    return match_count;
}

uint_fast8_t ncsh_autocompletions_first(const char* const search,
                                        const size_t search_length,
                                        char* match,
                                        struct ncsh_Autocompletion_Node* restrict tree,
                                        struct ncsh_Arena scratch_arena)
{
    struct ncsh_Autocompletion_Node* search_result = ncsh_autocompletions_search(search, search_length, tree);
    if (!search_result) {
        return 0;
    }

    struct ncsh_Autocompletion matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast8_t matches_count = ncsh_autocompletions_matches(matches, search_result, &scratch_arena);
    if (!matches_count) {
        return 0;
    }

    struct ncsh_Autocompletion potential_match = matches[0];
    for (uint_fast8_t i = 1; i < matches_count; ++i) {
        if (matches[i].weight > potential_match.weight) {
            potential_match = matches[i];
        }
    }

    memcpy(match, potential_match.value, NCSH_MAX_INPUT);

    return 1;
}
