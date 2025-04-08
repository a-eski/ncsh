/* Copyright ncsh by Alex Eski 2024 */
/* Based on eskilib_trie prefix tree implementation */

#pragma once

#include "../eskilib/eskilib_string.h"
#include "../ncsh_arena.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

// Forward Declaration: prefix tree for storing autocomplete posibilities
struct ncsh_Autocompletion_Node;

// Type Declaration: prefix tree for storing autocomplete possibilities
struct ncsh_Autocompletion_Node {
    bool is_end_of_a_word;
    uint_fast8_t weight;
    struct ncsh_Autocompletion_Node** nodes;
};

struct ncsh_Autocompletion {
    uint_fast8_t weight;
    char* value;
};

inline int ncsh_char_to_index(const char character)
{
    return (int)character - ' ';
}
inline char ncsh_index_to_char(const int index)
{
    return (char)index + ' ';
}

struct ncsh_Autocompletion_Node* ncsh_autocompletions_alloc(struct ncsh_Arena* const arena);

void ncsh_autocompletions_add(const char* const string,
                              const size_t length,
                              struct ncsh_Autocompletion_Node* restrict tree,
                              struct ncsh_Arena* const arena);

void ncsh_autocompletions_add_multiple(const struct eskilib_String* const strings,
                                       const int count,
                                       struct ncsh_Autocompletion_Node* restrict tree,
                                       struct ncsh_Arena* const arena);

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search(const char* const string,
                                                             const size_t length,
                                                             struct ncsh_Autocompletion_Node* restrict tree);

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search_string(const struct eskilib_String string,
                                                                    struct ncsh_Autocompletion_Node* restrict tree);

// gets all matches based on traversing the tree.
// populates matches into variable matches and returns 0 if no matches, number of matches length if any matches.
uint_fast8_t ncsh_autocompletions_get(const char* const search,
                                      const size_t search_length,
                                      struct ncsh_Autocompletion* matches,
                                      struct ncsh_Autocompletion_Node* restrict tree,
                                      struct ncsh_Arena scratch_arena);

// gets highest weighted match based on traversing the tree.
// populates match into variable match and returns 0 if not matches, 1 if any matches.
uint_fast8_t ncsh_autocompletions_first(const char* const search,
                                        const size_t search_length,
                                        char* match,
                                        struct ncsh_Autocompletion_Node* restrict tree,
                                        struct ncsh_Arena scratch_arena);
