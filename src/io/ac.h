/* Copyright ncsh (C) by Alex Eski 2024 */
/* ac.h: interact with prefix trie to manage autocompletions */
/* Based on eskilib etrie prefix trie implementation */
/* References the trie as 'tree' just because its easier to type... haha.. */

#pragma once

#include "../arena.h"
#include "../defines.h"
#include "../eskilib/str.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

/*  struct Autocompletion_Nodes
 *  Prefix tree (trie) for storing autocomplete possibilities
 *  Use stack for nodes, slightly faster than allocating on heap in benchmarks.
 */
typedef struct Autocompletion_Node_ {
    bool is_end_of_a_word;
    uint8_t weight;
    struct Autocompletion_Node_* nodes[NCSH_LETTERS];
} Autocompletion_Node;

/* struct Autocompletion
 * Used for storing results of autocompletion matching
 */
typedef struct {
    uint8_t weight;
    char* value;
} Autocompletion;

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
Autocompletion_Node* ac_alloc(Arena* rst arena);

void ac_add(char* rst string, size_t length, Autocompletion_Node* rst tree, Arena* rst arena);

void ac_add_multiple(Str* rst strings, int count, Autocompletion_Node* rst tree, Arena* rst arena);

Autocompletion_Node* ac_find(char* rst str, Autocompletion_Node* rst tree);

/* ac_get
 * Gets all matches based on traversing the trie.
 * Populates matches into variable matches.
 * Returns: number of matches (0 if no matches)
 */
uint8_t ac_get(char* rst search, Autocompletion* rst matches, Autocompletion_Node* rst tree, Arena scratch);

/* ac_first
 * Gets highest weighted match based on traversing the tree.
 * Populates match into variable match
 * Returns: 0 if no matches, 1 if any matches
 */
uint8_t ac_first(char* rst search, char* rst match, Autocompletion_Node* rst tree, Arena scratch);
