#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../eskilib/eskilib_string.h"
#include "../eskilib/eskilib_test.h"
#include "../ncsh_autocompletions.h"
#include "../ncsh_defines.h"

void ncsh_autocompletions_add_length_mismatch_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 3};
    ncsh_autocompletions_add(string.value, string.length, tree);

    // not crashing is a test pass here

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 4};
    ncsh_autocompletions_add(string.value, string.length, tree);

    // sanity check: unrelated letters are null
    eskilib_assert(tree->nodes[ncsh_char_to_index('b')] == NULL);
    eskilib_assert(tree->nodes[ncsh_char_to_index('c')] == NULL);
    eskilib_assert(tree->nodes[ncsh_char_to_index('d')] == NULL);
    eskilib_assert(tree->nodes[ncsh_char_to_index('e')] == NULL);

    struct ncsh_Autocompletion_Node* first_node = tree->nodes[ncsh_char_to_index('a')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);
    eskilib_assert(first_node->nodes[ncsh_char_to_index('b')] == NULL); // sanity check
    struct ncsh_Autocompletion_Node* second_node = first_node->nodes[ncsh_char_to_index('n')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);
    eskilib_assert(second_node->nodes[ncsh_char_to_index('b')] == NULL); // sanity check
    struct ncsh_Autocompletion_Node* third_node = second_node->nodes[ncsh_char_to_index('d')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == true);
    eskilib_assert(third_node->nodes[ncsh_char_to_index('b')] == NULL); // sanity check

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_duplicate_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 4};
    ncsh_autocompletions_add(string.value, string.length, tree);
    ncsh_autocompletions_add(string.value, string.length, tree);

    struct ncsh_Autocompletion_Node* first_node = tree->nodes[ncsh_char_to_index('a')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* second_node = first_node->nodes[ncsh_char_to_index('n')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* third_node = second_node->nodes[ncsh_char_to_index('d')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == true);
    eskilib_assert(third_node->weight == 2);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_multiple_unrelated_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "ls", .length = 3};
    ncsh_autocompletions_add(string_one.value, string_one.length, tree);
    struct eskilib_String string_two = {.value = "echo", .length = 5};
    ncsh_autocompletions_add(string_two.value, string_two.length, tree);

    struct ncsh_Autocompletion_Node* ls_first_node = tree->nodes[ncsh_char_to_index('l')];
    eskilib_assert(ls_first_node != NULL);
    eskilib_assert(ls_first_node->is_end_of_a_word == false);
    struct ncsh_Autocompletion_Node* ls_second_node = ls_first_node->nodes[ncsh_char_to_index('s')];
    eskilib_assert(ls_second_node != NULL);
    eskilib_assert(ls_second_node->is_end_of_a_word == true);

    struct ncsh_Autocompletion_Node* first_node = tree->nodes[ncsh_char_to_index('e')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* second_node = first_node->nodes[ncsh_char_to_index('c')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* third_node = second_node->nodes[ncsh_char_to_index('h')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* fourth_node = third_node->nodes[ncsh_char_to_index('o')];
    eskilib_assert(fourth_node != NULL);
    eskilib_assert(fourth_node->is_end_of_a_word == true);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_add_multiple_related_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    ncsh_autocompletions_add(string_one.value, string_one.length, tree);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    ncsh_autocompletions_add(string_two.value, string_two.length, tree);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    ncsh_autocompletions_add(string_three.value, string_three.length, tree);

    // gene
    struct ncsh_Autocompletion_Node* first_node = tree->nodes[ncsh_char_to_index('g')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* second_node = first_node->nodes[ncsh_char_to_index('e')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* third_node = second_node->nodes[ncsh_char_to_index('n')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* fourth_node = third_node->nodes[ncsh_char_to_index('e')];
    eskilib_assert(fourth_node != NULL);
    eskilib_assert(fourth_node->is_end_of_a_word == true);

    // genetic
    struct ncsh_Autocompletion_Node* genetic_fifth_node = fourth_node->nodes[ncsh_char_to_index('t')];
    eskilib_assert(genetic_fifth_node != NULL);
    eskilib_assert(genetic_fifth_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* genetic_sixth_node = genetic_fifth_node->nodes[ncsh_char_to_index('i')];
    eskilib_assert(genetic_sixth_node != NULL);
    eskilib_assert(genetic_sixth_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* genetic_seventh_node = genetic_sixth_node->nodes[ncsh_char_to_index('c')];
    eskilib_assert(genetic_seventh_node != NULL);
    eskilib_assert(genetic_seventh_node->is_end_of_a_word == true);

    // genius
    struct ncsh_Autocompletion_Node* genius_fourth_node = third_node->nodes[ncsh_char_to_index('i')];
    eskilib_assert(genius_fourth_node != NULL);
    eskilib_assert(genius_fourth_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* genius_fifth_node = genius_fourth_node->nodes[ncsh_char_to_index('u')];
    eskilib_assert(genius_fifth_node != NULL);
    eskilib_assert(genius_fifth_node->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* genius_sixth_node = genius_fifth_node->nodes[ncsh_char_to_index('s')];
    eskilib_assert(genius_sixth_node != NULL);
    eskilib_assert(genius_sixth_node->is_end_of_a_word == true);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    ncsh_autocompletions_add(string_one.value, string_one.length, tree);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    ncsh_autocompletions_add(string_two.value, string_two.length, tree);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    ncsh_autocompletions_add(string_three.value, string_three.length, tree);

    struct eskilib_String string_search = {.value = "gen", .length = 4};
    struct ncsh_Autocompletion_Node* result =
        ncsh_autocompletions_search(string_search.value, string_search.length, tree);
    eskilib_assert(result != NULL);
    struct ncsh_Autocompletion_Node* result_e = result->nodes[ncsh_char_to_index('e')];
    eskilib_assert(result_e != NULL);
    eskilib_assert(result_e->is_end_of_a_word == true);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_commands_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    ncsh_autocompletions_add("ls", 3, tree);
    ncsh_autocompletions_add("ls | sort", 10, tree);
    ncsh_autocompletions_add("ls | sort | wc -c", 18, tree);
    ncsh_autocompletions_add("ls > t.txt", 11, tree);
    ncsh_autocompletions_add("cat t.txt", 10, tree);
    ncsh_autocompletions_add("rm t.txt", 9, tree);
    ncsh_autocompletions_add("ss", 3, tree);

    struct ncsh_Autocompletion_Node* result = tree->nodes[ncsh_char_to_index('l')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == false);

    result = result->nodes[ncsh_char_to_index('s')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == true);

    result = result->nodes[ncsh_char_to_index(' ')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == false);

    struct ncsh_Autocompletion_Node* search_result =
        ncsh_autocompletions_search_string((struct eskilib_String){.value = "ls | ", .length = 6}, tree);
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[ncsh_char_to_index('s')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[ncsh_char_to_index('o')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[ncsh_char_to_index('r')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[ncsh_char_to_index('t')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == true);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_search_no_results_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    ncsh_autocompletions_add(string_one.value, string_one.length, tree);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    ncsh_autocompletions_add(string_two.value, string_two.length, tree);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    ncsh_autocompletions_add(string_three.value, string_three.length, tree);

    struct eskilib_String string_search = {.value = "ls", .length = 3};
    struct ncsh_Autocompletion_Node* search_result = ncsh_autocompletions_search_string(string_search, tree);
    eskilib_assert(search_result == NULL);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    ncsh_autocompletions_add("ls", 3, tree);
    ncsh_autocompletions_add("ls | wc -c", 11, tree);
    ncsh_autocompletions_add("ls | sort", 10, tree);
    ncsh_autocompletions_add("ls | sort | wc -c", 18, tree);
    ncsh_autocompletions_add("ls > t.txt", 11, tree);
    ncsh_autocompletions_add("cat t.txt", 10, tree);
    ncsh_autocompletions_add("rm t.txt", 9, tree);
    ncsh_autocompletions_add("ss", 3, tree);

    struct ncsh_Autocompletion_Node* search_result =
        ncsh_autocompletions_search_string((struct eskilib_String){.value = "ls | ", .length = 6}, tree);
    eskilib_assert(search_result != NULL);

    struct ncsh_Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = ncsh_autocompletions_get("ls | ", 6, autocomplete, tree);

    eskilib_assert(match_count == 3);
    eskilib_assert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    for (uint_fast32_t i = 0; i < NCSH_MAX_AUTOCOMPLETION_MATCHES - 1; i++) {
        if (autocomplete[i].value != NULL) {
            // printf("i:%lu %s\n", i, autocomplete[i]);
            free(autocomplete[i].value);
        }
    }
    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_no_results_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    ncsh_autocompletions_add("ls", 3, tree);
    ncsh_autocompletions_add("ls | wc -c", 11, tree);
    ncsh_autocompletions_add("ls | sort", 10, tree);
    ncsh_autocompletions_add("ls | sort | wc -c", 18, tree);
    ncsh_autocompletions_add("ls > t.txt", 11, tree);
    ncsh_autocompletions_add("cat t.txt", 10, tree);
    ncsh_autocompletions_add("rm t.txt", 9, tree);
    ncsh_autocompletions_add("ss", 3, tree);

    struct ncsh_Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = ncsh_autocompletions_get("n", 2, autocomplete, tree);

    eskilib_assert(match_count == 0);
    eskilib_assert(autocomplete[0].value == NULL);

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_multiple_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    ncsh_autocompletions_add("ls", 3, tree);
    ncsh_autocompletions_add("ls | wc -c", 11, tree);
    ncsh_autocompletions_add("ls | sort", 10, tree);
    ncsh_autocompletions_add("ls | sort | wc -c", 18, tree);
    ncsh_autocompletions_add("ls > t.txt", 11, tree);
    ncsh_autocompletions_add("cat t.txt", 10, tree);
    ncsh_autocompletions_add("rm t.txt", 9, tree);
    ncsh_autocompletions_add("ss", 3, tree);

    struct ncsh_Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = ncsh_autocompletions_get("ls | ", 6, autocomplete, tree);

    eskilib_assert(match_count == 3);
    eskilib_assert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    for (uint_fast32_t i = 0; i < NCSH_MAX_AUTOCOMPLETION_MATCHES - 1; i++) {
        if (autocomplete[i].value != NULL) {
            // printf("i:%lu %s\n", i, autocomplete[i]);
            free(autocomplete[i].value);
        }
    }

    struct ncsh_Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    match_count = ncsh_autocompletions_get("l", 2, autocomplete_two, tree);

    eskilib_assert(match_count == 5);
    eskilib_assert(memcmp(autocomplete_two[0].value, "s", 2) == 0);
    eskilib_assert(memcmp(autocomplete_two[1].value, "s > t.txt", 10) == 0);
    eskilib_assert(memcmp(autocomplete_two[2].value, "s | sort", 9) == 0);
    eskilib_assert(memcmp(autocomplete_two[3].value, "s | sort | wc -c", 17) == 0);
    eskilib_assert(memcmp(autocomplete_two[4].value, "s | wc -c", 10) == 0);

    for (uint_fast32_t i = 0; i < NCSH_MAX_AUTOCOMPLETION_MATCHES - 1; i++) {
        if (autocomplete_two[i].value != NULL) {
            // printf("i:%lu %s\n", i, autocomplete_two[i]);
            free(autocomplete_two[i].value);
        }
    }

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_matches_multiple_simulation_test(void)
{
    struct ncsh_Autocompletion_Node* tree = ncsh_autocompletions_malloc();
    eskilib_assert(tree != NULL);

    ncsh_autocompletions_add("ls", 3, tree);
    ncsh_autocompletions_add("ls | wc -c", 11, tree);
    ncsh_autocompletions_add("ls | sort", 10, tree);
    ncsh_autocompletions_add("ls | sort | wc -c", 18, tree);
    ncsh_autocompletions_add("ls > t.txt", 11, tree);
    ncsh_autocompletions_add("cat t.txt", 10, tree);
    ncsh_autocompletions_add("rm t.txt", 9, tree);
    ncsh_autocompletions_add("ss", 3, tree);
    ncsh_autocompletions_add("nvim", 5, tree);
    ncsh_autocompletions_add("nvim .", 7, tree);

    struct ncsh_Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = ncsh_autocompletions_get("l", 2, autocomplete, tree);

    eskilib_assert(match_count == 5);
    eskilib_assert(memcmp(autocomplete[0].value, "s", 2) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "s > t.txt", 10) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "s | sort", 9) == 0);
    eskilib_assert(memcmp(autocomplete[3].value, "s | sort | wc -c", 17) == 0);
    eskilib_assert(memcmp(autocomplete[4].value, "s | wc -c", 10) == 0);

    for (uint_fast32_t i = 0; i < NCSH_MAX_AUTOCOMPLETION_MATCHES - 1; i++) {
        if (autocomplete[i].value != NULL) {
            // printf("i:%lu %s\n", i, autocomplete[i]);
            free(autocomplete[i].value);
        }
    }

    struct ncsh_Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    match_count = ncsh_autocompletions_get("ls", 3, autocomplete_two, tree);

    eskilib_assert(match_count == 4);
    eskilib_assert(memcmp(autocomplete_two[0].value, " > t.txt", 9) == 0);
    eskilib_assert(memcmp(autocomplete_two[1].value, " | sort", 8) == 0);
    eskilib_assert(memcmp(autocomplete_two[2].value, " | sort | wc -c", 16) == 0);
    eskilib_assert(memcmp(autocomplete_two[3].value, " | wc -c", 8) == 0);

    for (uint_fast32_t i = 0; i < NCSH_MAX_AUTOCOMPLETION_MATCHES - 1; i++) {
        if (autocomplete_two[i].value != NULL) {
            // printf("i:%lu %s\n", i, autocomplete_two[i]);
            free(autocomplete_two[i].value);
        }
    }

    ncsh_autocompletions_free(tree);
}

void ncsh_autocompletions_tests(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_autocompletions_add_length_mismatch_test", ncsh_autocompletions_add_length_mismatch_test);
    eskilib_test_run("ncsh_autocompletions_add_test", ncsh_autocompletions_add_test);
    eskilib_test_run("ncsh_autocompletions_add_duplicate_test", ncsh_autocompletions_add_duplicate_test);
    eskilib_test_run("ncsh_autocompletions_add_multiple_unrelated_test",
                     ncsh_autocompletions_add_multiple_unrelated_test);
    eskilib_test_run("ncsh_autocompletions_add_multiple_related_test", ncsh_autocompletions_add_multiple_related_test);
    eskilib_test_run("ncsh_autocompletions_search_test", ncsh_autocompletions_search_test);
    eskilib_test_run("ncsh_autocompletions_search_no_results_test", ncsh_autocompletions_search_no_results_test);
    eskilib_test_run("ncsh_autocompletions_search_commands_test", ncsh_autocompletions_search_commands_test);
    eskilib_test_run("ncsh_autocompletions_matches_test", ncsh_autocompletions_matches_test);
    eskilib_test_run("ncsh_autocompletions_matches_no_results_test", ncsh_autocompletions_matches_no_results_test);
    eskilib_test_run("ncsh_autocompletions_matches_multiple_test", ncsh_autocompletions_matches_multiple_test);
    eskilib_test_run("ncsh_autocompletions_matches_multiple_simulation_test",
                     ncsh_autocompletions_matches_multiple_simulation_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_autocompletions_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
