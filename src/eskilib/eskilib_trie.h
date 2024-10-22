#ifndef eskilib_trie_h
#define eskilib_trie_h

#include "stdbool.h"

#include "eskilib_string.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127
#define NCSH_MATCH_LENGTH 100 // max length of a match

//prefix tree for storing autocomplete posibilities
struct eskilib_Trie;

struct eskilib_Trie {
	bool is_end_of_a_word;
	char letter;
	struct eskilib_Trie* nodes[NCSH_LETTERS];
};

struct eskilib_Trie_Matches {
	uint_fast32_t count;
	struct eskilib_String* entries;
};

struct eskilib_Trie* eskilib_trie_malloc();

void eskilib_trie_free(struct eskilib_Trie* tree);

void eskilib_trie_add(struct eskilib_String string, struct eskilib_Trie* tree);

//returns the node that matches the string or null
struct eskilib_Trie* eskilib_trie_search(struct eskilib_String string, struct eskilib_Trie* tree);

//returns all potential matches
// void eskilib_trie_get_matches(struct eskilib_String string, struct eskilib_Trie_Matches* matches, struct eskilib_Trie* tree);

int eskilib_trie_map_char(char character);

char eskilib_trie_map_position(int position);

#endif // !eskilib_trie_h

