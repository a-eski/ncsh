// Copyright (c) ncsh by Alex Eski 2024
// Based on ncsh_autocompletions prefix tree implementation.

#ifndef ncsh_autocompletions_h
#define ncsh_autocompletions_h

#include <stdint.h>
#include <stdbool.h>

#include "eskilib/eskilib_string.h"

#define NCSH_LETTERS 96 // ascii printable characters 32-127

// prefix tree for storing autocomplete posibilities
struct ncsh_Autocompletions;

struct ncsh_Autocompletions {
	bool is_end_of_a_word;
	char letter;
	struct ncsh_Autocompletions* nodes[NCSH_LETTERS];
};

struct ncsh_Autocompletions_Matches {
	uint_fast32_t count;
	struct eskilib_String* entries;
};

struct ncsh_Autocompletions* ncsh_autocompletions_malloc();

void ncsh_autocompletions_free(struct ncsh_Autocompletions* tree);

void ncsh_autocompletions_add(char* string, uint_fast32_t length, struct ncsh_Autocompletions* tree);
void ncsh_autocompletions_add_string(struct eskilib_String string, struct ncsh_Autocompletions* tree);
void ncsh_autocompletions_add_multiple(struct eskilib_String* strings, uint_fast32_t count, struct ncsh_Autocompletions* tree);

// gets all matches based on a search into matches. returns matches_length;
uint_fast32_t ncsh_autocompletions_get(char* search,
				       uint_fast32_t search_length,
				       char* matches[],
				       const uint_fast32_t max_match_length,
				       struct ncsh_Autocompletions* tree);

int ncsh_autocompletions_map_char(char character);
char ncsh_autocompletions_map_position(int position);

#endif // !ncsh_autocompletions_h

