#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "eskilib_trie.h"
#include "eskilib_string.h"

struct eskilib_Trie* eskilib_trie_malloc() {
	struct eskilib_Trie* tree = calloc(1, sizeof(struct eskilib_Trie));
	if (tree == NULL)
		return NULL;

	tree->is_end_of_a_word = false;
	return tree;
}

void eskilib_trie_free(struct eskilib_Trie* tree) {
	for (uint_fast32_t i = 0; i < NCSH_LETTERS; i++) {
		if (tree->nodes[i] != NULL)
			eskilib_trie_free(tree->nodes[i]);
		else
			continue;
	}
	free(tree);
}

void eskilib_trie_add(struct eskilib_String string, struct eskilib_Trie* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; i++) { //string.length - 1 because it includes null terminator
		index = (int)string.value[i] - 'a';
		if (tree->nodes[index] == NULL) {
			tree->nodes[index] = calloc(1, sizeof(struct eskilib_Trie));
			tree->nodes[index]->is_end_of_a_word = false;
		}

		tree = tree->nodes[index];
	}

	tree->is_end_of_a_word = true;
}

struct eskilib_Trie* eskilib_trie_search(struct eskilib_String string, struct eskilib_Trie* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; i++) {
		index = (int)string.value[i] - 'a';
		if (tree->nodes[index] == NULL)
			return NULL;

		tree = tree->nodes[index];
	}

	if (tree != NULL)
		return tree;

	return NULL;
}

static const char position_map[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
	'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

char eskilib_trie_map_position(int position) {
	return position_map[position];
}

int eskilib_trie_map_char(char character) {
	return (int)character - 'a';
}

