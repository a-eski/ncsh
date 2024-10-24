#include <stdint.h>
#include <stdlib.h>

#include "../ncsh_autocompletions.h"
#include "../eskilib/eskilib_test.h"
#include "../eskilib/eskilib_string.h"

void ncsh_autocompletions_add_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	ncsh_autocompletions_add(string.value, string.length, tree);

	// sanity check: unrelated letters are null
	eskilib_assert(tree->nodes[ncsh_autocompletions_map_char('b')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocompletions_map_char('c')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocompletions_map_char('d')] == NULL);
	eskilib_assert(tree->nodes[ncsh_autocompletions_map_char('e')] == NULL);

	struct ncsh_Autocompletions* first_node = tree->nodes[ncsh_autocompletions_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->letter == 'a')
	eskilib_assert(first_node->nodes[ncsh_autocompletions_map_char('b')] == NULL); //sanity check
	struct ncsh_Autocompletions* second_node = first_node->nodes[ncsh_autocompletions_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->letter == 'n')
	eskilib_assert(second_node->nodes[ncsh_autocompletions_map_char('b')] == NULL); //sanity check
	struct ncsh_Autocompletions* third_node = second_node->nodes[ncsh_autocompletions_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);
	eskilib_assert(third_node->letter == 'd');
	eskilib_assert(third_node->nodes[ncsh_autocompletions_map_char('b')] == NULL); //sanity check

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_duplicate_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string = { .value = "and", .length = 4 };
	ncsh_autocompletions_add(string.value, string.length, tree);
	ncsh_autocompletions_add_string(string, tree);

	struct ncsh_Autocompletions* first_node = tree->nodes[ncsh_autocompletions_map_char('a')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->letter == 'a')

	struct ncsh_Autocompletions* second_node = first_node->nodes[ncsh_autocompletions_map_char('n')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->letter == 'n');

	struct ncsh_Autocompletions* third_node = second_node->nodes[ncsh_autocompletions_map_char('d')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == true);
	eskilib_assert(third_node->letter == 'd');

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_multiple_unrelated_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "ls", .length = 3 };
	ncsh_autocompletions_add(string_one.value, string_one.length, tree);
	struct eskilib_String string_two = { .value = "echo", .length = 5 };
	ncsh_autocompletions_add_string(string_two, tree);

	struct ncsh_Autocompletions* ls_first_node = tree->nodes[ncsh_autocompletions_map_char('l')];
	eskilib_assert(ls_first_node != NULL);
	eskilib_assert(ls_first_node->is_end_of_a_word == false);
	eskilib_assert(ls_first_node->letter == 'l');
	struct ncsh_Autocompletions* ls_second_node = ls_first_node->nodes[ncsh_autocompletions_map_char('s')];
	eskilib_assert(ls_second_node != NULL);
	eskilib_assert(ls_second_node->is_end_of_a_word == true);
	eskilib_assert(ls_second_node->letter == 's');

	struct ncsh_Autocompletions* first_node = tree->nodes[ncsh_autocompletions_map_char('e')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->letter == 'e');
	struct ncsh_Autocompletions* second_node = first_node->nodes[ncsh_autocompletions_map_char('c')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->letter == 'c');
	struct ncsh_Autocompletions* third_node = second_node->nodes[ncsh_autocompletions_map_char('h')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	eskilib_assert(third_node->letter == 'h');
	struct ncsh_Autocompletions* fourth_node = third_node->nodes[ncsh_autocompletions_map_char('o')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);
	eskilib_assert(fourth_node->letter == 'o');

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_multiple_related_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	ncsh_autocompletions_add(string_one.value, string_one.length, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	ncsh_autocompletions_add_string(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	ncsh_autocompletions_add_string(string_three, tree);

	// gene
	struct ncsh_Autocompletions* first_node = tree->nodes[ncsh_autocompletions_map_char('g')];
	eskilib_assert(first_node != NULL);
	eskilib_assert(first_node->is_end_of_a_word == false);
	eskilib_assert(first_node->letter == 'g');
	struct ncsh_Autocompletions* second_node = first_node->nodes[ncsh_autocompletions_map_char('e')];
	eskilib_assert(second_node != NULL);
	eskilib_assert(second_node->is_end_of_a_word == false);
	eskilib_assert(second_node->letter == 'e');
	struct ncsh_Autocompletions* third_node = second_node->nodes[ncsh_autocompletions_map_char('n')];
	eskilib_assert(third_node != NULL);
	eskilib_assert(third_node->is_end_of_a_word == false);
	eskilib_assert(third_node->letter == 'n');
	struct ncsh_Autocompletions* fourth_node = third_node->nodes[ncsh_autocompletions_map_char('e')];
	eskilib_assert(fourth_node != NULL);
	eskilib_assert(fourth_node->is_end_of_a_word == true);
	eskilib_assert(fourth_node->letter == 'e');

	// genetic
	struct ncsh_Autocompletions* genetic_fifth_node = fourth_node->nodes[ncsh_autocompletions_map_char('t')];
	eskilib_assert(genetic_fifth_node  != NULL);
	eskilib_assert(genetic_fifth_node->is_end_of_a_word == false);
	eskilib_assert(genetic_fifth_node->letter == 't');
	struct ncsh_Autocompletions* genetic_sixth_node = genetic_fifth_node->nodes[ncsh_autocompletions_map_char('i')];
	eskilib_assert(genetic_sixth_node != NULL);
	eskilib_assert(genetic_sixth_node->is_end_of_a_word == false);
	eskilib_assert(genetic_sixth_node->letter == 'i');
	struct ncsh_Autocompletions* genetic_seventh_node = genetic_sixth_node->nodes[ncsh_autocompletions_map_char('c')];
	eskilib_assert(genetic_seventh_node != NULL);
	eskilib_assert(genetic_seventh_node->is_end_of_a_word == true);
	eskilib_assert(genetic_seventh_node->letter == 'c');

	// genius
	struct ncsh_Autocompletions* genius_fourth_node = third_node->nodes[ncsh_autocompletions_map_char('i')];
	eskilib_assert(genius_fourth_node != NULL);
	eskilib_assert(genius_fourth_node->is_end_of_a_word == false);
	eskilib_assert(genius_fourth_node->letter == 'i');
	struct ncsh_Autocompletions* genius_fifth_node = genius_fourth_node->nodes[ncsh_autocompletions_map_char('u')];
	eskilib_assert(genius_fifth_node != NULL);
	eskilib_assert(genius_fifth_node->is_end_of_a_word == false);
	eskilib_assert(genius_fifth_node->letter == 'u');
	struct ncsh_Autocompletions* genius_sixth_node = genius_fifth_node->nodes[ncsh_autocompletions_map_char('s')];
	eskilib_assert(genius_sixth_node != NULL);
	eskilib_assert(genius_sixth_node->is_end_of_a_word == true);
	eskilib_assert(genius_sixth_node->letter == 's');

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	ncsh_autocompletions_add(string_one.value, string_one.length, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	ncsh_autocompletions_add(string_two.value, string_two.length, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	ncsh_autocompletions_add_string(string_three, tree);

	struct eskilib_String string_search = { .value = "gen", .length = 4 };
	struct ncsh_Autocompletions* result = ncsh_autocompletions_search(string_search.value, string_search.length, tree);
	eskilib_assert(result != NULL);
	struct ncsh_Autocompletions* result_e = result->nodes[ncsh_autocompletions_map_char('e')];
	eskilib_assert(result_e != NULL);
	eskilib_assert(result_e->is_end_of_a_word == true);
	eskilib_assert(result_e->letter == 'e');

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_commands_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls", .length = 3 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort | wc -c", .length = 18 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls > t.txt", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "cat t.txt", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "rm t.txt", .length = 9 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ss", .length = 3 }, tree);

	struct ncsh_Autocompletions* result = tree->nodes[ncsh_autocompletions_map_char('l')];
	eskilib_assert(result != NULL);
	eskilib_assert(result->is_end_of_a_word == false);
	eskilib_assert(result->letter == 'l');
	result = result->nodes[ncsh_autocompletions_map_char('s')];
	eskilib_assert(result != NULL);
	eskilib_assert(result->is_end_of_a_word == true);
	eskilib_assert(result->letter == 's');
	result = result->nodes[ncsh_autocompletions_map_char(' ')];
	eskilib_assert(result != NULL);
	eskilib_assert(result->is_end_of_a_word == false);
	eskilib_assert(result->letter == ' ');

	struct ncsh_Autocompletions* search_result = ncsh_autocompletions_search_string((struct eskilib_String){ .value = "ls | ", .length = 6 }, tree);
	eskilib_assert(search_result != NULL);
	eskilib_assert(search_result->is_end_of_a_word == false);
	eskilib_assert(search_result->letter == ' ');
	search_result = search_result->nodes[ncsh_autocompletions_map_char('s')];
	eskilib_assert(search_result != NULL);
	eskilib_assert(search_result->is_end_of_a_word == false);
	eskilib_assert(search_result->letter == 's');
	search_result = search_result->nodes[ncsh_autocompletions_map_char('o')];
	eskilib_assert(search_result != NULL);
	eskilib_assert(search_result->is_end_of_a_word == false);
	eskilib_assert(search_result->letter == 'o');
	search_result = search_result->nodes[ncsh_autocompletions_map_char('r')];
	eskilib_assert(search_result != NULL);
	eskilib_assert(search_result->is_end_of_a_word == false);
	eskilib_assert(search_result->letter == 'r');
	search_result = search_result->nodes[ncsh_autocompletions_map_char('t')];
	eskilib_assert(search_result != NULL);
	eskilib_assert(search_result->is_end_of_a_word == true);
	eskilib_assert(search_result->letter == 't');

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_no_results_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	struct eskilib_String string_one = { .value = "gene", .length = 5 };
	ncsh_autocompletions_add_string(string_one, tree);
	struct eskilib_String string_two = { .value = "genetic", .length = 8 };
	ncsh_autocompletions_add_string(string_two, tree);
	struct eskilib_String string_three = { .value = "genius", .length = 7 };
	ncsh_autocompletions_add_string(string_three, tree);

	struct eskilib_String string_search = { .value = "ls", .length = 3 };
	struct ncsh_Autocompletions* search_result = ncsh_autocompletions_search_string(string_search, tree);
	eskilib_assert(search_result == NULL);

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls", .length = 3 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | wc -c", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort | wc -c", .length = 18 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls > t.txt", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "cat t.txt", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "rm t.txt", .length = 9 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ss", .length = 3 }, tree);

	struct ncsh_Autocompletions* search_result = ncsh_autocompletions_search_string((struct eskilib_String){ .value = "ls | ", .length = 6 }, tree);
	eskilib_assert(search_result != NULL);

	constexpr uint_fast32_t max_match_length = 256;
	char* autocomplete[max_match_length] = {0};

	uint_fast32_t match_count = ncsh_autocompletions_get("ls | ", 6, autocomplete, max_match_length, tree);

	eskilib_assert(match_count == 3);
	eskilib_assert(eskilib_string_equals(autocomplete[0], "sort", 5) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[1], "sort | wc -c", 13) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[2], "wc -c", 6) == true);

	for (uint_fast32_t i = 0; i < max_match_length - 1; i++) {
		if (autocomplete[i] != NULL) {
			// printf("i:%lu %s\n", i, autocomplete[i]);
			free(autocomplete[i]);
		}
	}
	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_no_results_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls", .length = 3 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | wc -c", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort | wc -c", .length = 18 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls > t.txt", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "cat t.txt", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "rm t.txt", .length = 9 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ss", .length = 3 }, tree);

	constexpr uint_fast32_t max_match_length = 256;
	char* autocomplete[max_match_length] = {0};

	uint_fast32_t match_count = ncsh_autocompletions_get("l", 2, autocomplete, max_match_length, tree);


	eskilib_assert(match_count == 0);
	eskilib_assert(autocomplete[0] == NULL);

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_multiple_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls", .length = 3 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | wc -c", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort | wc -c", .length = 18 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls > t.txt", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "cat t.txt", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "rm t.txt", .length = 9 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ss", .length = 3 }, tree);

	constexpr uint_fast32_t max_match_length = 256;
	char* autocomplete[max_match_length] = {0};

	uint_fast32_t match_count = ncsh_autocompletions_get("ls | ", 6, autocomplete, max_match_length, tree);

	eskilib_assert(match_count == 3);
	eskilib_assert(eskilib_string_equals(autocomplete[0], "sort", 5) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[1], "sort | wc -c", 13) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[2], "wc -c", 6) == true);

	for (uint_fast32_t i = 0; i < max_match_length - 1; i++) {
		if (autocomplete[i] != NULL) {
			// printf("i:%lu %s\n", i, autocomplete[i]);
			free(autocomplete[i]);
		}
	}

	char* autocomplete_two[max_match_length] = {0};

	match_count = ncsh_autocompletions_get("l", 2, autocomplete_two, max_match_length, tree);

	eskilib_assert(match_count == 5);
	eskilib_assert(eskilib_string_equals(autocomplete_two[0], "s", 2) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[1], "s > t.txt", 10) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[2], "s | sort", 9) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[3], "s | sort | wc -c", 17) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[4], "s | wc -c", 10) == true);

	for (uint_fast32_t i = 0; i < max_match_length - 1; i++) {
		if (autocomplete_two[i] != NULL) {
			// printf("i:%lu %s\n", i, autocomplete_two[i]);
			free(autocomplete_two[i]);
		}
	}

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_multiple_simulation_test(void) {
	struct ncsh_Autocompletions* tree = ncsh_autocompletions_malloc();
	eskilib_assert(tree != NULL);

	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls", .length = 3 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | wc -c", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls | sort | wc -c", .length = 18 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ls > t.txt", .length = 11 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "cat t.txt", .length = 10 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "rm t.txt", .length = 9 }, tree);
	ncsh_autocompletions_add_string((struct eskilib_String){ .value = "ss", .length = 3 }, tree);
	ncsh_autocompletions_add("nvim", 5, tree);
	ncsh_autocompletions_add("nvim .", 7, tree);

	constexpr uint_fast32_t max_match_length = 256;
	char* autocomplete[max_match_length] = {0};

	uint_fast32_t match_count = ncsh_autocompletions_get("l", 2, autocomplete, max_match_length, tree);

	eskilib_assert(match_count == 5);
	eskilib_assert(eskilib_string_equals(autocomplete[0], "s", 2) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[1], "s > t.txt", 10) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[2], "s | sort", 9) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[3], "s | sort | wc -c", 17) == true);
	eskilib_assert(eskilib_string_equals(autocomplete[4], "s | wc -c", 10) == true);
	for (uint_fast32_t i = 0; i < max_match_length - 1; i++) {
		if (autocomplete[i] != NULL) {
			// printf("i:%lu %s\n", i, autocomplete[i]);
			free(autocomplete[i]);
		}
	}

	char* autocomplete_two[max_match_length] = {0};

	match_count = ncsh_autocompletions_get("ls", 3, autocomplete_two, max_match_length, tree);

	eskilib_assert(match_count == 4);
	eskilib_assert(eskilib_string_equals(autocomplete_two[1], " > t.txt", 10) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[2], " | sort", 9) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[3], " | sort | wc -c", 17) == true);
	eskilib_assert(eskilib_string_equals(autocomplete_two[4], " | wc -c", 10) == true);

	for (uint_fast32_t i = 0; i < max_match_length - 1; i++) {
		if (autocomplete_two[i] != NULL) {
			// printf("i:%lu %s\n", i, autocomplete_two[i]);
			free(autocomplete_two[i]);
		}
	}

	ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_tests(void) {
	eskilib_test_run("ncsh_autocompletions_add_test", ncsh_autocompletions_add_test);
	eskilib_test_run("ncsh_autocompletions_add_duplicate_test", ncsh_autocompletions_add_duplicate_test);
	eskilib_test_run("ncsh_autocompletions_add_multiple_unrelated_test", ncsh_autocompletions_add_multiple_unrelated_test);
	eskilib_test_run("ncsh_autocompletions_add_multiple_related_test", ncsh_autocompletions_add_multiple_related_test);
	eskilib_test_run("ncsh_autocompletions_search_test", ncsh_autocompletions_search_test);
	eskilib_test_run("ncsh_autocompletions_search_no_results_test", ncsh_autocompletions_search_no_results_test);
	eskilib_test_run("ncsh_autocompletions_search_commands_test", ncsh_autocompletions_search_commands_test);
	eskilib_test_run("eskilib_tree_matches_test", ncsh_autocompletions_matches_test);
	eskilib_test_run("ncsh_autocompletions_matches_no_results_test", ncsh_autocompletions_matches_no_results_test);
	eskilib_test_run("eskilib_tree_matches_multiple_test", ncsh_autocompletions_matches_multiple_test);
}

#ifndef ncsh_TEST_ALL
int main(void) {
	ncsh_autocompletions_tests();

	return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */

