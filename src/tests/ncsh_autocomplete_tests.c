#include <stdint.h>
#include <stdlib.h>

#include "../ncsh_autocomplete.h"
#include "../eskilib/eskilib_test.h"
#include "../eskilib/eskilib_string.h"

void ncsh_autocomplete_add_test(void) {
	struct ncsh_Autocomplete* tree = ncsh_autocomplete_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	ncsh_autocomplete_add(string, tree);

	// sanity check: unrelated letters are null
	eskilib_assert(tree->nodes[ncsh_autocomplete_map_char('b')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocomplete_map_char('c')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocomplete_map_char('d')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocomplete_map_char('e')] == NULL);

	struct ncsh_Autocomplete* first_node = tree->nodes[ncsh_autocomplete_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->nodes[ncsh_autocomplete_map_char('b')] == NULL); //sanity check
	struct ncsh_Autocomplete* second_node = first_node->nodes[ncsh_autocomplete_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->nodes[ncsh_autocomplete_map_char('b')] == NULL); //sanity check
	struct ncsh_Autocomplete* third_node = second_node->nodes[ncsh_autocomplete_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);
	eskilib_assert(third_node->nodes[ncsh_autocomplete_map_char('b')] == NULL); //sanity check

	ncsh_autocomplete_free(tree);
}

void ncsh_autocomplete_add_duplicate_test(void) {
	struct ncsh_Autocomplete* tree = ncsh_autocomplete_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	ncsh_autocomplete_add(string, tree);
	ncsh_autocomplete_add(string, tree);

	struct ncsh_Autocomplete* first_node = tree->nodes[ncsh_autocomplete_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);

	struct ncsh_Autocomplete* second_node = first_node->nodes[ncsh_autocomplete_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);

	struct ncsh_Autocomplete* third_node = second_node->nodes[ncsh_autocomplete_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);

	ncsh_autocomplete_free(tree);
}

void ncsh_autocomplete_add_multiple_unrelated_test(void) {
	struct ncsh_Autocomplete* tree = ncsh_autocomplete_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "ls", .length = 3 };
	ncsh_autocomplete_add(string_one, tree);
	struct eskilib_String string_two = { .value = "echo", .length = 5 };
	ncsh_autocomplete_add(string_two, tree);

	struct ncsh_Autocomplete* ls_first_node = tree->nodes[ncsh_autocomplete_map_char('l')];
	eskilib_assert(ls_first_node != NULL);
	eskilib_assert(ls_first_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* ls_second_node = ls_first_node->nodes[ncsh_autocomplete_map_char('s')];
	eskilib_assert(ls_second_node != NULL);
	eskilib_assert(ls_second_node->is_end_of_a_word == true);

	struct ncsh_Autocomplete* first_node = tree->nodes[ncsh_autocomplete_map_char('e')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* second_node = first_node->nodes[ncsh_autocomplete_map_char('c')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* third_node = second_node->nodes[ncsh_autocomplete_map_char('h')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* fourth_node = third_node->nodes[ncsh_autocomplete_map_char('o')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);

	ncsh_autocomplete_free(tree);
}

void ncsh_autocomplete_add_multiple_related_test(void) {
	struct ncsh_Autocomplete* tree = ncsh_autocomplete_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	ncsh_autocomplete_add(string_one, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	ncsh_autocomplete_add(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	ncsh_autocomplete_add(string_three, tree);

	// gene
	struct ncsh_Autocomplete* first_node = tree->nodes[ncsh_autocomplete_map_char('g')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* second_node = first_node->nodes[ncsh_autocomplete_map_char('e')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* third_node = second_node->nodes[ncsh_autocomplete_map_char('n')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* fourth_node = third_node->nodes[ncsh_autocomplete_map_char('e')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);

	// genetic
	struct ncsh_Autocomplete* genetic_fifth_node = fourth_node->nodes[ncsh_autocomplete_map_char('t')];
	eskilib_assert(genetic_fifth_node  != NULL);
	eskilib_assert(genetic_fifth_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* genetic_sixth_node = genetic_fifth_node->nodes[ncsh_autocomplete_map_char('i')];
	eskilib_assert(genetic_sixth_node != NULL);
	eskilib_assert(genetic_sixth_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* genetic_seventh_node = genetic_sixth_node->nodes[ncsh_autocomplete_map_char('c')];
	eskilib_assert(genetic_seventh_node != NULL);
	eskilib_assert(genetic_seventh_node->is_end_of_a_word == true);

	// genius
	struct ncsh_Autocomplete* genius_fourth_node = third_node->nodes[ncsh_autocomplete_map_char('i')];
	eskilib_assert(genius_fourth_node != NULL);
	eskilib_assert(genius_fourth_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* genius_fifth_node = genius_fourth_node->nodes[ncsh_autocomplete_map_char('u')];
	eskilib_assert(genius_fifth_node != NULL);
	eskilib_assert(genius_fifth_node->is_end_of_a_word == false);
	struct ncsh_Autocomplete* genius_sixth_node = genius_fifth_node->nodes[ncsh_autocomplete_map_char('s')];
	eskilib_assert(genius_sixth_node != NULL);
	eskilib_assert(genius_sixth_node->is_end_of_a_word == true);

	ncsh_autocomplete_free(tree);
}

void ncsh_autocomplete_search_test(void) {
	struct ncsh_Autocomplete* tree = ncsh_autocomplete_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	ncsh_autocomplete_add(string_one, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	ncsh_autocomplete_add(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	ncsh_autocomplete_add(string_three, tree);

	struct eskilib_String string_search = { .value = "gen", .length = 4 };
	struct ncsh_Autocomplete* result = ncsh_autocomplete_search(string_search, tree);
	eskilib_assert(result != NULL);
	struct ncsh_Autocomplete* result_e = result->nodes[ncsh_autocomplete_map_char('e')];
	eskilib_assert(result_e != NULL);
	eskilib_assert(result_e->is_end_of_a_word == true);

	ncsh_autocomplete_free(tree);
}

void ncsh_autocomplete_tests(void) {
	eskilib_test_run("ncsh_autocomplete_add_test", ncsh_autocomplete_add_test);
	eskilib_test_run("ncsh_autocomplete_add_duplicate_test", ncsh_autocomplete_add_duplicate_test);
	eskilib_test_run("ncsh_autocomplete_add_multiple_unrelated_test", ncsh_autocomplete_add_multiple_unrelated_test);
	eskilib_test_run("ncsh_autocomplete_add_multiple_related_test", ncsh_autocomplete_add_multiple_related_test);
	eskilib_test_run("ncsh_autocomplete_search_test", ncsh_autocomplete_search_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_autocomplete_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
