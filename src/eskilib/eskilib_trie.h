#ifndef eskilib_trie_h
#define eskilib_trie_h

#include "stdbool.h"

#include "eskilib_string.h"
#include <stdint.h>

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

void eskilib_trie_add(char* string, uint_fast32_t length, struct eskilib_Trie* tree);
void eskilib_trie_add_string(struct eskilib_String string, struct eskilib_Trie* tree);
void eskilib_trie_add_multiple(struct eskilib_String* strings, uint_fast32_t count, struct eskilib_Trie* tree);

//returns the node that matches the string or null
struct eskilib_Trie* eskilib_trie_search(char* string, uint_fast32_t length, struct eskilib_Trie* tree);
struct eskilib_Trie* eskilib_trie_search_string(struct eskilib_String string, struct eskilib_Trie* tree);

// returns all potential matches into matches. returns matches_length;
uint_fast32_t eskilib_trie_matches(char* matches[], struct eskilib_Trie* tree);

// gets all matches based on a search into matches. returns matches_length;
uint_fast32_t eskilib_trie_get(char* search, uint_fast32_t search_length, char* matches[], struct eskilib_Trie* tree);

int eskilib_trie_map_char(char character);
char eskilib_trie_map_position(int position);

#endif // !eskilib_trie_h

