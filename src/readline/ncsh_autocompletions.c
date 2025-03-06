// Copyright (c) ncsh by Alex Eski 2025

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

struct ncsh_Autocompletion_Node* ncsh_autocompletions_malloc(void)
{
    struct ncsh_Autocompletion_Node* tree = calloc(1, sizeof(struct ncsh_Autocompletion_Node));
    if (!tree) {
        return NULL;
    }

    tree->is_end_of_a_word = false;
    return tree;
}

void ncsh_autocompletions_free(struct ncsh_Autocompletion_Node* tree)
{
    assert(tree);
    if (!tree) {
        return;
    }

    for (uint_fast8_t i = 0; i < NCSH_LETTERS; ++i) {
        if (tree->nodes[i]) {
            ncsh_autocompletions_free(tree->nodes[i]);
        }
    }
    free(tree);
}

void ncsh_autocompletions_free_matches(struct ncsh_Autocompletion* matches, uint_fast8_t matches_count)
{
    assert(matches);
    if (!matches) {
        return;
    }

    for (uint_fast8_t i = 0; i <= matches_count; ++i) {
        free(matches[i].value);
    }
}

void ncsh_autocompletions_add(char* string, size_t length, struct ncsh_Autocompletion_Node* tree)
{
    assert(string);
    assert(length > 0);
    assert(tree);
    if (!string || !length || !tree || length > NCSH_MAX_INPUT) {
        return;
    }

    int index = 0;

    for (size_t i = 0; i < length - 1; ++i) { // string.length - 1 because it includes null terminator
        index = ncsh_char_to_index(string[i]);
        if (index < 0 || index > 96) {
            continue;
        }

        if (!tree->nodes[index]) {
            tree->nodes[index] = calloc(1, sizeof(struct ncsh_Autocompletion_Node));
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

void ncsh_autocompletions_add_multiple(struct eskilib_String* strings, int count, struct ncsh_Autocompletion_Node* tree)
{
    if (!strings || count <= 0 || !tree) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        ncsh_autocompletions_add(strings[i].value, strings[i].length, tree);
    }
}

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search(char* string, size_t length,
                                                             struct ncsh_Autocompletion_Node* tree)
{
    assert(string);
    assert(length > 0);
    assert(tree);
    if (!string || !length || !tree) {
        return NULL;
    }

    int index = 0;

    for (size_t i = 0; i < length - 1; ++i) {
        index = ncsh_char_to_index(string[i]);

        if (!tree->nodes[index]) {
            return NULL;
        }

        tree = tree->nodes[index];
    }

    return tree;
}

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search_string(struct eskilib_String string,
                                                                    struct ncsh_Autocompletion_Node* tree)
{
    assert(string.value);
    assert(string.length > 0);
    assert(tree);
    if (!string.value || !string.length || !tree) {
        return NULL;
    }

    int index = 0;

    for (size_t i = 0; i < string.length - 1; ++i) {
        index = ncsh_char_to_index(string.value[i]);

        if (!tree->nodes[index]) {
            return NULL;
        }

        tree = tree->nodes[index];
    }

    return tree;
}

void ncsh_autocompletions_match(struct ncsh_Autocompletion* matches, uint_fast32_t* string_position,
                                uint_fast8_t* matches_position, struct ncsh_Autocompletion_Node* tree)
{
    for (int i = 0; i < NCSH_LETTERS; ++i) {
        if (tree->nodes[i]) {
            if (*matches_position + 1 >= NCSH_MAX_AUTOCOMPLETION_MATCHES) {
                return;
            }

            if (!matches[*matches_position].value) {
                matches[*matches_position].value = malloc(NCSH_MAX_INPUT);
                if (!matches[*matches_position].value) {
                    return;
                }

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

            ncsh_autocompletions_match(matches, string_position, matches_position, tree->nodes[i]);

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

uint_fast8_t ncsh_autocompletions_matches(struct ncsh_Autocompletion* matches, struct ncsh_Autocompletion_Node* tree)
{
    uint_fast32_t string_position = 0;
    uint_fast8_t matches_position = 0;

    ncsh_autocompletions_match(matches, &string_position, &matches_position, tree);

    return matches_position;
}

uint_fast8_t ncsh_autocompletions_get(char* search, size_t search_length, struct ncsh_Autocompletion* matches,
                                      struct ncsh_Autocompletion_Node* tree)
{
    struct ncsh_Autocompletion_Node* search_result = ncsh_autocompletions_search(search, search_length, tree);
    if (!search_result) {
        return 0;
    }

    uint_fast8_t match_count = ncsh_autocompletions_matches(matches, search_result);
    if (!match_count) {
        return 0;
    }

    return match_count;
}

uint_fast8_t ncsh_autocompletions_first(char* search, size_t search_length, char* match,
                                        struct ncsh_Autocompletion_Node* tree)
{
    struct ncsh_Autocompletion_Node* search_result = ncsh_autocompletions_search(search, search_length, tree);
    if (!search_result) {
        return 0;
    }

    struct ncsh_Autocompletion matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast8_t matches_count = ncsh_autocompletions_matches(matches, search_result);
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
    ncsh_autocompletions_free_matches(matches, matches_count);

    return 1;
}
