#include <stdint.h>
#include <stdlib.h>

#include "../eskilib_trie.h"
#include "../eskilib_test.h"
#include "../eskilib_string.h"

void eskilib_trie_add_test(void) {
	struct eskilib_Trie* tree = eskilib_trie_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	eskilib_trie_add(string, tree);

	// sanity check: unrelated letters are null
	eskilib_assert(tree->nodes[eskilib_trie_map_char('b')] == NULL);
	eskilib_assert(tree->nodes[eskilib_trie_map_char('c')] == NULL);
	eskilib_assert(tree->nodes[eskilib_trie_map_char('d')] == NULL);
	eskilib_assert(tree->nodes[eskilib_trie_map_char('e')] == NULL);

	struct eskilib_Trie* first_node = tree->nodes[eskilib_trie_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->nodes[eskilib_trie_map_char('b')] == NULL); //sanity check
	struct eskilib_Trie* second_node = first_node->nodes[eskilib_trie_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->nodes[eskilib_trie_map_char('b')] == NULL); //sanity check
	struct eskilib_Trie* third_node = second_node->nodes[eskilib_trie_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);
	eskilib_assert(third_node->nodes[eskilib_trie_map_char('b')] == NULL); //sanity check

	eskilib_trie_free(tree);
}

void eskilib_trie_add_duplicate_test(void) {
	struct eskilib_Trie* tree = eskilib_trie_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	eskilib_trie_add(string, tree);
	eskilib_trie_add(string, tree);

	struct eskilib_Trie* first_node = tree->nodes[eskilib_trie_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);

	struct eskilib_Trie* second_node = first_node->nodes[eskilib_trie_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);

	struct eskilib_Trie* third_node = second_node->nodes[eskilib_trie_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);

	eskilib_trie_free(tree);
}

void eskilib_trie_add_multiple_unrelated_test(void) {
	struct eskilib_Trie* tree = eskilib_trie_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "ls", .length = 3 };
	eskilib_trie_add(string_one, tree);
	struct eskilib_String string_two = { .value = "echo", .length = 5 };
	eskilib_trie_add(string_two, tree);

	struct eskilib_Trie* ls_first_node = tree->nodes[eskilib_trie_map_char('l')];
	eskilib_assert(ls_first_node != NULL);
	eskilib_assert(ls_first_node->is_end_of_a_word == false);
	struct eskilib_Trie* ls_second_node = ls_first_node->nodes[eskilib_trie_map_char('s')];
	eskilib_assert(ls_second_node != NULL);
	eskilib_assert(ls_second_node->is_end_of_a_word == true);

	struct eskilib_Trie* first_node = tree->nodes[eskilib_trie_map_char('e')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	struct eskilib_Trie* second_node = first_node->nodes[eskilib_trie_map_char('c')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	struct eskilib_Trie* third_node = second_node->nodes[eskilib_trie_map_char('h')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	struct eskilib_Trie* fourth_node = third_node->nodes[eskilib_trie_map_char('o')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);

	eskilib_trie_free(tree);
}

void eskilib_trie_add_multiple_related_test(void) {
	struct eskilib_Trie* tree = eskilib_trie_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	eskilib_trie_add(string_one, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	eskilib_trie_add(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	eskilib_trie_add(string_three, tree);

	// gene
	struct eskilib_Trie* first_node = tree->nodes[eskilib_trie_map_char('g')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	struct eskilib_Trie* second_node = first_node->nodes[eskilib_trie_map_char('e')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	struct eskilib_Trie* third_node = second_node->nodes[eskilib_trie_map_char('n')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	struct eskilib_Trie* fourth_node = third_node->nodes[eskilib_trie_map_char('e')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);

	// genetic
	struct eskilib_Trie* genetic_fifth_node = fourth_node->nodes[eskilib_trie_map_char('t')];
	eskilib_assert(genetic_fifth_node  != NULL);
	eskilib_assert(genetic_fifth_node->is_end_of_a_word == false);
	struct eskilib_Trie* genetic_sixth_node = genetic_fifth_node->nodes[eskilib_trie_map_char('i')];
	eskilib_assert(genetic_sixth_node != NULL);
	eskilib_assert(genetic_sixth_node->is_end_of_a_word == false);
	struct eskilib_Trie* genetic_seventh_node = genetic_sixth_node->nodes[eskilib_trie_map_char('c')];
	eskilib_assert(genetic_seventh_node != NULL);
	eskilib_assert(genetic_seventh_node->is_end_of_a_word == true);

	// genius
	struct eskilib_Trie* genius_fourth_node = third_node->nodes[eskilib_trie_map_char('i')];
	eskilib_assert(genius_fourth_node != NULL);
	eskilib_assert(genius_fourth_node->is_end_of_a_word == false);
	struct eskilib_Trie* genius_fifth_node = genius_fourth_node->nodes[eskilib_trie_map_char('u')];
	eskilib_assert(genius_fifth_node != NULL);
	eskilib_assert(genius_fifth_node->is_end_of_a_word == false);
	struct eskilib_Trie* genius_sixth_node = genius_fifth_node->nodes[eskilib_trie_map_char('s')];
	eskilib_assert(genius_sixth_node != NULL);
	eskilib_assert(genius_sixth_node->is_end_of_a_word == true);

	eskilib_trie_free(tree);
}

void eskilib_trie_search_test(void) {
	struct eskilib_Trie* tree = eskilib_trie_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	eskilib_trie_add(string_one, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	eskilib_trie_add(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	eskilib_trie_add(string_three, tree);

	struct eskilib_String string_search = { .value = "gen", .length = 4 };
	struct eskilib_Trie* result = eskilib_trie_search(string_search, tree);
	eskilib_assert(result != NULL);
	struct eskilib_Trie* result_e = result->nodes[eskilib_trie_map_char('e')];
	eskilib_assert(result_e != NULL);
	eskilib_assert(result_e->is_end_of_a_word == true);

	eskilib_trie_free(tree);
}

void eskilib_trie_tests(void) {
	eskilib_test_run("eskilib_trie_add_test", eskilib_trie_add_test);
	eskilib_test_run("eskilib_trie_add_duplicate_test", eskilib_trie_add_duplicate_test);
	eskilib_test_run("eskilib_trie_add_multiple_unrelated_test", eskilib_trie_add_multiple_unrelated_test);
	eskilib_test_run("eskilib_trie_add_multiple_related_test", eskilib_trie_add_multiple_related_test);
	eskilib_test_run("eskilib_trie_search_test", eskilib_trie_search_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	eskilib_trie_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
