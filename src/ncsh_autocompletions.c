#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "ncsh_autocompletions.h"
#include "eskilib/eskilib_string.h"

struct ncsh_Autocompletions* ncsh_autocompletions_malloc() {
	struct ncsh_Autocompletions* tree = calloc(1, sizeof(struct ncsh_Autocompletions));
	if (tree == NULL)
		return NULL;

	tree->is_end_of_a_word = false;
	return tree;
}

void ncsh_autocompletions_free(struct ncsh_Autocompletions* tree) {
	assert(tree != NULL);
	if (tree == NULL)
		return;

	for (uint_fast32_t i = 0; i < NCSH_LETTERS; ++i) {
		if (tree->nodes[i] != NULL)
			ncsh_autocompletions_free(tree->nodes[i]);
		else
			continue;
	}
	free(tree);
}

void ncsh_autocompletions_free_values(char **autocompletions, uint_fast32_t count) {
	assert(autocompletions != NULL);
	if (autocompletions == NULL)
		return;

	for (uint_fast32_t i = 0; i < count; ++i) {
		if (autocompletions[i] != NULL) {
			free(autocompletions[i]);
		}
	}
}

void ncsh_autocompletions_add(char* string, uint_fast32_t length, struct ncsh_Autocompletions* tree) {
	assert(string != NULL);
	assert(length > 0);
	assert(tree != NULL);
	if (string == NULL || length == 0 || tree == NULL)
		return;

	int index = 0;

	for (uint_fast32_t i = 0; i < length - 1; ++i) { //string.length - 1 because it includes null terminator
		index = (int)string[i] - ' ';
		if (index < 32 || index > 127) //found unsupported character
			break;

		if (tree->nodes[index] == NULL) {
			tree->nodes[index] = calloc(1, sizeof(struct ncsh_Autocompletions));
			tree->nodes[index]->is_end_of_a_word = false;
			tree->nodes[index]->letter = string[i];
		}

		tree = tree->nodes[index];
	}

	tree->is_end_of_a_word = true;
}

void ncsh_autocompletions_add_string(struct eskilib_String string, struct ncsh_Autocompletions* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);
	assert(tree != NULL);
	if (string.value == NULL || string.length == 0 || tree == NULL)
		return;

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; ++i) { //string.length - 1 because it includes null terminator
		index = (int)string.value[i] - ' ';
		if (tree->nodes[index] == NULL) {
			tree->nodes[index] = calloc(1, sizeof(struct ncsh_Autocompletions));
			tree->nodes[index]->is_end_of_a_word = false;
			tree->nodes[index]->letter = string.value[i];
		}

		tree = tree->nodes[index];
	}

	tree->is_end_of_a_word = true;
}

void ncsh_autocompletions_add_multiple(struct eskilib_String* strings, uint_fast32_t count, struct ncsh_Autocompletions* tree) {
	assert(strings != NULL);
	assert(count > 0);
	assert(tree != NULL);
	if (strings == NULL || count == 0 || tree == NULL)
		return;

	for (uint_fast32_t i = 0; i < count; ++i) {
		ncsh_autocompletions_add_string(strings[i], tree);
	}
}

struct ncsh_Autocompletions* ncsh_autocompletions_search(char* string, uint_fast32_t length, struct ncsh_Autocompletions* tree) {
	assert(string != NULL);
	assert(length > 0);
	assert(tree != NULL);
	if (string == NULL || length == 0 || tree == NULL)
		return NULL;

	int index = 0;

	for (uint_fast32_t i = 0; i < length - 1; ++i) {
		index = (int)string[i] - ' ';
		if (tree->nodes[index] == NULL)
			return NULL;

		tree = tree->nodes[index];
	}

	if (tree != NULL)
		return tree;

	return NULL;
}

struct ncsh_Autocompletions* ncsh_autocompletions_search_string(struct eskilib_String string, struct ncsh_Autocompletions* tree) {
	assert(string.value != NULL);
	assert(string.length > 0);
	assert(tree != NULL);
	if (string.value == NULL || string.length == 0 || tree == NULL)
		return NULL;

	int index = 0;

	for (uint_fast32_t i = 0; i < string.length - 1; ++i) {
		index = (int)string.value[i] - ' ';
		if (tree->nodes[index] == NULL)
			return NULL;

		tree = tree->nodes[index];
	}

	if (tree != NULL)
		return tree;

	return NULL;
}

void ncsh_autocompletions_match(char* matches[],
				uint_fast32_t* string_position,
				uint_fast32_t* matches_position,
				const uint_fast32_t max_match_length,
				struct ncsh_Autocompletions* tree) {
	for (uint_fast32_t i = 0; i < NCSH_LETTERS; ++i) {
		if (tree->nodes[i] != NULL) {
			if (matches[*matches_position] == NULL) {
				matches[*matches_position] = malloc(sizeof(char) * max_match_length);
				if (matches[*matches_position] == NULL)
					return;

				if (*string_position > 0 && *matches_position > 0)
					eskilib_string_copy(matches[*matches_position], matches[*matches_position - 1], *string_position);
			}

			matches[*matches_position][*string_position] = tree->nodes[i]->letter;
			++*string_position;
			matches[*matches_position][*string_position] = '\0';

			if (tree->nodes[i]->is_end_of_a_word == true && *matches_position + 1 < max_match_length)
				++*matches_position;

			ncsh_autocompletions_match(matches, string_position, matches_position, max_match_length, tree->nodes[i]);

			if (matches[*matches_position] != NULL)
				++*matches_position;

			*string_position = *string_position - 1;
		}
	}
}

uint_fast32_t ncsh_autocompletions_matches(char* matches[], const uint_fast32_t max_match_length, struct ncsh_Autocompletions* tree) {
	uint_fast32_t string_position = 0;
	uint_fast32_t matches_position = 0;

	ncsh_autocompletions_match(matches, &string_position, &matches_position, max_match_length, tree);

	return matches_position;
}

uint_fast32_t ncsh_autocompletions_get(char* search,
				       uint_fast32_t search_length,
				       char* matches[],
				       const uint_fast32_t max_match_length,
				       struct ncsh_Autocompletions* tree) {
	struct ncsh_Autocompletions *search_result = ncsh_autocompletions_search(search, search_length, tree);
	if (search_result == NULL)
		return 0;

	return ncsh_autocompletions_matches(matches, max_match_length, search_result);
}

int ncsh_autocompletions_map_char(char character) {
	return (int)character - ' ';
}

char ncsh_autocompletions_map_position(int position) {
	return (char)position + ' ';
}

