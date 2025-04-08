/* Copyright ncsh by Alex Eski 2024 */
/* Based on eskilib_trie prefix tree implementation */

#pragma once

#include "../eskilib/eskilib_string.h"
#include "../arena.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

// Forward Declaration: prefix tree for storing autocomplete posibilities
struct Autocompletion_Node;

/*  struct Autocompletion_Nodes
 *  Prefix tree (trie) for storing autocomplete possibilities
 *  Use stack for nodes, slightly faster than allocating on heap in benchmarks.
 */
struct Autocompletion_Node {
    bool is_end_of_a_word;
    uint_fast8_t weight;
    struct Autocompletion_Node* nodes[NCSH_LETTERS];
};

/* struct Autocompletion
 * Used for storing results of autocompletion matching
 */
struct Autocompletion {
    uint_fast8_t weight;
    char* value;
};

static inline int char_to_index(const char character)
{
    return (int)character - ' ';
}
static inline char index_to_char(const int index)
{
    return (char)index + ' ';
}

struct Autocompletion_Node* autocompletions_alloc(struct Arena* const arena);

void autocompletions_add(const char* const string,
                              const size_t length,
                              struct Autocompletion_Node* restrict tree,
                              struct Arena* const arena);

void autocompletions_add_multiple(const struct eskilib_String* const strings,
                                       const int count,
                                       struct Autocompletion_Node* restrict tree,
                                       struct Arena* const arena);

struct Autocompletion_Node* autocompletions_search(const char* const string,
                                                             const size_t length,
                                                             struct Autocompletion_Node* restrict tree);

struct Autocompletion_Node* autocompletions_search_string(const struct eskilib_String string,
                                                                    struct Autocompletion_Node* restrict tree);

/* autocompletions_get
 * Gets all matches based on traversing the trie.
 * Populates matches into variable matches.
 * Returns: number of matches (0 if no matches)
 */
uint_fast8_t autocompletions_get(const char* const search,
                                      const size_t search_length,
                                      struct Autocompletion* matches,
                                      struct Autocompletion_Node* restrict tree,
                                      struct Arena scratch_arena);

/* autocompletions_first
 * Gets highest weighted match based on traversing the tree.
 * Populates match into variable match
 * Returns: 0 if no matches, 1 if any matches
 */
uint_fast8_t autocompletions_first(const char* const search,
                                        const size_t search_length,
                                        char* match,
                                        struct Autocompletion_Node* restrict tree,
                                        struct Arena scratch_arena);
