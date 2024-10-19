#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "ncsh_autocomplete.h"
#include "eskilib/eskilib_string.h"

struct ncsh_Autocomplete* ncsh_autocomplete_malloc() {
	struct ncsh_Autocomplete* tree = calloc(1, sizeof(struct ncsh_Autocomplete));
	if (tree == NULL)
		return NULL;

	tree->is_end_of_a_word = false;
	return tree;
}

void ncsh_autocomplete_free(struct ncsh_Autocomplete* tree) {
	for (uint_fast32_t i = 0; i < NCSH_LETTERS; i++) {
		if (tree->nodes[i] != NULL)
			ncsh_autocomplete_free(tree->nodes[i]);
		else
			continue;
	}
	free(tree);
}

void ncsh_autocomplete_add(struct eskilib_String string, struct ncsh_Autocomplete* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; i++) { //string.length - 1 because it includes null terminator
		index = (int)string.value[i] - 'a';
		if (tree->nodes[index] == NULL) {
			tree->nodes[index] = calloc(1, sizeof(struct ncsh_Autocomplete));
			tree->nodes[index]->is_end_of_a_word = false;
		}

		tree = tree->nodes[index];
	}

	tree->is_end_of_a_word = true;
}

struct ncsh_Autocomplete* ncsh_autocomplete_search(struct eskilib_String string) {
	assert(string.value != NULL);
	assert(string.length > 0);

	return NULL;
}

static const char position_map[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

char ncsh_autocomplete_map_position(int position) {
	return position_map[position];
}

int ncsh_autocomplete_map_char(char character) {
	return (int)character - 'a';
}

