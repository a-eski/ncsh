#ifndef eskilib_trie_h
#define eskilib_trie_h

#include "stdbool.h"

#include "eskilib_string.h"

#define NCSH_LETTERS 26

//prefix tree for storing autocomplete posibilities
struct eskilib_Trie;

struct eskilib_Trie {
	bool is_end_of_a_word;
	struct eskilib_Trie* nodes[NCSH_LETTERS];
};

struct eskilib_Trie* eskilib_trie_malloc();

void eskilib_trie_free(struct eskilib_Trie* tree);

void eskilib_trie_add(struct eskilib_String string, struct eskilib_Trie* tree);

struct eskilib_Trie* eskilib_trie_search(struct eskilib_String string, struct eskilib_Trie* tree);

// char eskilib_trie_map_position(int position);

int eskilib_trie_map_char(char character);

#endif // !eskilib_trie_h

