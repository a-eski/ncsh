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
		index = (int)string.value[i] - ' ';
		if (tree->nodes[index] == NULL) {
			tree->nodes[index] = calloc(1, sizeof(struct eskilib_Trie));
			tree->nodes[index]->is_end_of_a_word = false;
			tree->nodes[index]->letter = string.value[i];
		}

		tree = tree->nodes[index];
	}

	tree->is_end_of_a_word = true;
}

struct eskilib_Trie* eskilib_trie_search(struct eskilib_String string, struct eskilib_Trie* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);
	assert(tree != NULL);

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; i++) {
		index = (int)string.value[i] - ' ';
		if (tree->nodes[index] == NULL)
			return NULL;

		tree = tree->nodes[index];
	}

	if (tree != NULL)
		return tree;

	return NULL;
}

/*void eskilib_trie_match(char* matches[], uint_fast32_t* array_position, uint_fast32_t* position, struct eskilib_Trie* tree) {
	for (uint_fast32_t i = 0; i < NCSH_LETTERS; i++) {
		if (tree->nodes[i] != NULL) {
			if (matches[*array_position] == NULL) {
				matches[*array_position] = malloc(sizeof(char) * NCSH_MATCH_LENGTH);

				if (*position > 0 && *array_position > 0)
					eskilib_string_copy(matches[*array_position], matches[*array_position - 1], *position);
			}

			matches[*array_position][*position] = tree->nodes[i]->letter;
			++*position;
			matches[*array_position][*position] = '\0';

			if (tree->nodes[i]->is_end_of_a_word == true && *array_position + 1 < NCSH_MATCH_LENGTH)
				++*array_position;

			eskilib_trie_match(matches, array_position, position, tree->nodes[i]);
		}
	}
}

void eskilib_trie_matches(char* matches[], struct eskilib_Trie* tree) {
	uint_fast32_t array_position = 0;
	uint_fast32_t position = 0;

	for (uint_fast32_t i = 0; i < NCSH_LETTERS; i++) {
		if (tree->nodes[i] != NULL) {
			if (matches[array_position] == NULL) {
				matches[array_position] = malloc(sizeof(char) * NCSH_MATCH_LENGTH);

				if (position > 0 && array_position > 0)
					eskilib_string_copy(matches[array_position], matches[array_position - 1], position);
			}

			matches[array_position][position] = tree->nodes[i]->letter;
			++position;
			matches[array_position][position] = '\0';

			if (tree->nodes[i]->is_end_of_a_word == true && array_position + 1 < NCSH_MATCH_LENGTH)
				++array_position;

			eskilib_trie_match(matches, &array_position, &position, tree->nodes[i]);

			if (matches[array_position] != NULL)
				++array_position;

			position = 0;
		}
	}
}*/

int eskilib_trie_map_char(char character) {
	return (int)character - ' ';
}

char eskilib_trie_map_position(int position) {
	return (char)position + ' ';
}

