#ifndef ncsh_autocomplete_h
#define ncsh_autocomplete_h

#include "stdbool.h"

#include "eskilib/eskilib_string.h"

#define NCSH_LETTERS 26

//prefix tree for storing autocomplete posibilities
struct ncsh_Autocomplete;

struct ncsh_Autocomplete {
	bool is_end_of_a_word;
	struct ncsh_Autocomplete* nodes[NCSH_LETTERS];
};

struct ncsh_Autocomplete* ncsh_autocomplete_malloc();

void ncsh_autocomplete_free(struct ncsh_Autocomplete* tree);

void ncsh_autocomplete_add(struct eskilib_String string, struct ncsh_Autocomplete* tree);

struct ncsh_Autocomplete* ncsh_autocomplete_search(struct eskilib_String string, struct ncsh_Autocomplete* tree);

// char ncsh_autocomplete_map_position(int position);

int ncsh_autocomplete_map_char(char character);

#endif // !ncsh_autocomplete_h

