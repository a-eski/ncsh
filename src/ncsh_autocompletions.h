// Copyright (c) ncsh by Alex Eski 2024
// Based on eskilib_trie prefix tree implementation.

#ifndef ncsh_autocompletions_h
#define ncsh_autocompletions_h

#include "eskilib/eskilib_string.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

// prefix tree for storing autocomplete posibilities
struct ncsh_Autocompletion_Node;

struct ncsh_Autocompletion_Node {
	bool is_end_of_a_word;
	uint_fast8_t weight;
	struct ncsh_Autocompletion_Node* nodes[NCSH_LETTERS];
};

struct ncsh_Autocompletion {
	uint_fast8_t weight;
	char* value;
};

inline int ncsh_char_to_index(char character) { return (int)character - ' '; }
inline char ncsh_index_to_char(int index) { return (char)index + ' '; }

struct ncsh_Autocompletion_Node* ncsh_autocompletions_malloc();

void ncsh_autocompletions_free(struct ncsh_Autocompletion_Node* tree);

void ncsh_autocompletions_add(char* string, size_t length, struct ncsh_Autocompletion_Node* tree);
void ncsh_autocompletions_add_multiple(struct eskilib_String* strings, uint_fast32_t count, struct ncsh_Autocompletion_Node* tree);

struct ncsh_Autocompletion_Node* ncsh_autocompletions_search(char* string, size_t length, struct ncsh_Autocompletion_Node* tree);
struct ncsh_Autocompletion_Node* ncsh_autocompletions_search_string(struct eskilib_String string, struct ncsh_Autocompletion_Node* tree);

// gets all matches based on traversing the tree.
// populates matches into variable matches and returns 0 if no matches, number of matches length if any matches.
uint_fast8_t ncsh_autocompletions_get(char* search,
				       size_t search_length,
				       struct ncsh_Autocompletion* matches,
				       struct ncsh_Autocompletion_Node* tree);

// gets highest weighted match based on traversing the tree.
// populates match into variable match and returns 0 if not matches, 1 if any matches.
uint_fast8_t ncsh_autocompletions_first(char* search,
				       size_t search_length,
				       char* match,
				       struct ncsh_Autocompletion_Node* tree);

#endif // !ncsh_autocompletions_h

