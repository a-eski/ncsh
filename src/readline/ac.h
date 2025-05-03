/* Copyright ncsh (C) by Alex Eski 2024 */
/* ac.h: interact with prefix trie to manage autocompletions */
/* Based on eskilib etrie prefix trie implementation */
/* References the trie as 'tree' just because its easier to type... haha.. */

#pragma once

#include "../arena.h"
#include "../eskilib/estr.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

/*  struct Autocompletion_Nodes
 *  Prefix tree (trie) for storing autocomplete possibilities
 *  Use stack for nodes, slightly faster than allocating on heap in benchmarks.
 */
struct Autocompletion_Node {
    bool is_end_of_a_word;
    uint8_t weight;
    struct Autocompletion_Node* nodes[NCSH_LETTERS];
};

/* struct Autocompletion
 * Used for storing results of autocompletion matching
 */
struct Autocompletion {
    uint8_t weight;
    char* value;
};

static inline int char_to_index(char character)
{
    return (int)character - ' ';
}
static inline char index_to_char(int index)
{
    return (char)index + ' ';
}

/* ac_alloc
 * Allocates the root of the trie using the passed in arena.
 * Returns: the root of the trie
 */
struct Autocompletion_Node* ac_alloc(struct Arena* restrict arena);

void ac_add(char* restrict string, size_t length, struct Autocompletion_Node* restrict tree,
            struct Arena* restrict arena);

void ac_add_multiple(struct estr* restrict strings, int count, struct Autocompletion_Node* restrict tree,
                     struct Arena* restrict arena);

struct Autocompletion_Node* ac_find(char* restrict str, struct Autocompletion_Node* restrict tree);

/* ac_get
 * Gets all matches based on traversing the trie.
 * Populates matches into variable matches.
 * Returns: number of matches (0 if no matches)
 */
uint8_t ac_get(char* restrict search, struct Autocompletion* restrict matches,
               struct Autocompletion_Node* restrict tree, struct Arena scratch);

/* ac_first
 * Gets highest weighted match based on traversing the tree.
 * Populates match into variable match
 * Returns: 0 if no matches, 1 if any matches
 */
uint8_t ac_first(char* restrict search, char* restrict match, struct Autocompletion_Node* restrict tree,
                 struct Arena scratch);
