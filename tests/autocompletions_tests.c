#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../src/defines.h"
#include "../src/eskilib/eskilib_string.h"
#include "../src/eskilib/eskilib_test.h"
#include "../src/readline/autocompletions.h"
#include "lib/arena_test_helper.h"

void autocompletions_add_length_mismatch_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 3};
    autocompletions_add(string.value, string.length, tree, &arena);

    // not crashing is a test pass here

    ARENA_TEST_TEARDOWN;
}

void autocompletions_add_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 4};
    autocompletions_add(string.value, string.length, tree, &arena);

    // sanity check: unrelated letters are null
    eskilib_assert(tree->nodes[char_to_index('b')] == NULL);
    eskilib_assert(tree->nodes[char_to_index('c')] == NULL);
    eskilib_assert(tree->nodes[char_to_index('d')] == NULL);
    eskilib_assert(tree->nodes[char_to_index('e')] == NULL);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);
    eskilib_assert(first_node->nodes[char_to_index('b')] == NULL); // sanity check
    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);
    eskilib_assert(second_node->nodes[char_to_index('b')] == NULL); // sanity check
    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == true);
    eskilib_assert(third_node->nodes[char_to_index('b')] == NULL); // sanity check

    ARENA_TEST_TEARDOWN;
}

void autocompletions_add_duplicate_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string = {.value = "and", .length = 4};
    autocompletions_add(string.value, string.length, tree, &arena);
    autocompletions_add(string.value, string.length, tree, &arena);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == true);
    eskilib_assert(third_node->weight == 2);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_add_multiple_unrelated_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "ls", .length = 3};
    autocompletions_add(string_one.value, string_one.length, tree, &arena);
    struct eskilib_String string_two = {.value = "echo", .length = 5};
    autocompletions_add(string_two.value, string_two.length, tree, &arena);

    struct Autocompletion_Node* ls_first_node = tree->nodes[char_to_index('l')];
    eskilib_assert(ls_first_node != NULL);
    eskilib_assert(ls_first_node->is_end_of_a_word == false);
    struct Autocompletion_Node* ls_second_node = ls_first_node->nodes[char_to_index('s')];
    eskilib_assert(ls_second_node != NULL);
    eskilib_assert(ls_second_node->is_end_of_a_word == true);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('e')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('c')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('h')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == false);

    struct Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('o')];
    eskilib_assert(fourth_node != NULL);
    eskilib_assert(fourth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_add_multiple_related_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    autocompletions_add(string_one.value, string_one.length, tree, &arena);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    autocompletions_add(string_two.value, string_two.length, tree, &arena);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    autocompletions_add(string_three.value, string_three.length, tree, &arena);

    // gene
    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('g')];
    eskilib_assert(first_node != NULL);
    eskilib_assert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('e')];
    eskilib_assert(second_node != NULL);
    eskilib_assert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('n')];
    eskilib_assert(third_node != NULL);
    eskilib_assert(third_node->is_end_of_a_word == false);

    struct Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('e')];
    eskilib_assert(fourth_node != NULL);
    eskilib_assert(fourth_node->is_end_of_a_word == true);

    // genetic
    struct Autocompletion_Node* genetic_fifth_node = fourth_node->nodes[char_to_index('t')];
    eskilib_assert(genetic_fifth_node != NULL);
    eskilib_assert(genetic_fifth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genetic_sixth_node = genetic_fifth_node->nodes[char_to_index('i')];
    eskilib_assert(genetic_sixth_node != NULL);
    eskilib_assert(genetic_sixth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genetic_seventh_node = genetic_sixth_node->nodes[char_to_index('c')];
    eskilib_assert(genetic_seventh_node != NULL);
    eskilib_assert(genetic_seventh_node->is_end_of_a_word == true);

    // genius
    struct Autocompletion_Node* genius_fourth_node = third_node->nodes[char_to_index('i')];
    eskilib_assert(genius_fourth_node != NULL);
    eskilib_assert(genius_fourth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genius_fifth_node = genius_fourth_node->nodes[char_to_index('u')];
    eskilib_assert(genius_fifth_node != NULL);
    eskilib_assert(genius_fifth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genius_sixth_node = genius_fifth_node->nodes[char_to_index('s')];
    eskilib_assert(genius_sixth_node != NULL);
    eskilib_assert(genius_sixth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_search_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    autocompletions_add(string_one.value, string_one.length, tree, &arena);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    autocompletions_add(string_two.value, string_two.length, tree, &arena);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    autocompletions_add(string_three.value, string_three.length, tree, &arena);

    struct eskilib_String string_search = {.value = "gen", .length = 4};
    struct Autocompletion_Node* result = autocompletions_search(string_search.value, string_search.length, tree);
    eskilib_assert(result != NULL);
    struct Autocompletion_Node* result_e = result->nodes[char_to_index('e')];
    eskilib_assert(result_e != NULL);
    eskilib_assert(result_e->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_search_commands_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", 3, tree, &arena);
    autocompletions_add("ls | sort", 10, tree, &arena);
    autocompletions_add("ls | sort | wc -c", 18, tree, &arena);
    autocompletions_add("ls > t.txt", 11, tree, &arena);
    autocompletions_add("cat t.txt", 10, tree, &arena);
    autocompletions_add("rm t.txt", 9, tree, &arena);
    autocompletions_add("ss", 3, tree, &arena);

    struct Autocompletion_Node* result = tree->nodes[char_to_index('l')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == false);

    result = result->nodes[char_to_index('s')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == true);

    result = result->nodes[char_to_index(' ')];
    eskilib_assert(result != NULL);
    eskilib_assert(result->is_end_of_a_word == false);

    struct Autocompletion_Node* search_result =
        autocompletions_search_string((struct eskilib_String){.value = "ls | ", .length = 6}, tree);
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[char_to_index('s')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[char_to_index('o')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[char_to_index('r')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == false);

    search_result = search_result->nodes[char_to_index('t')];
    eskilib_assert(search_result != NULL);
    eskilib_assert(search_result->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_search_no_results_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    struct eskilib_String string_one = {.value = "gene", .length = 5};
    autocompletions_add(string_one.value, string_one.length, tree, &arena);
    struct eskilib_String string_two = {.value = "genetic", .length = 8};
    autocompletions_add(string_two.value, string_two.length, tree, &arena);
    struct eskilib_String string_three = {.value = "genius", .length = 7};
    autocompletions_add(string_three.value, string_three.length, tree, &arena);

    struct eskilib_String string_search = {.value = "ls", .length = 3};
    struct Autocompletion_Node* search_result = autocompletions_search_string(string_search, tree);
    eskilib_assert(search_result == NULL);

    ARENA_TEST_TEARDOWN;
}

void autocompletions_matches_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", 3, tree, &arena);
    autocompletions_add("ls | wc -c", 11, tree, &arena);
    autocompletions_add("ls | sort", 10, tree, &arena);
    autocompletions_add("ls | sort | wc -c", 18, tree, &arena);
    autocompletions_add("ls > t.txt", 11, tree, &arena);
    autocompletions_add("cat t.txt", 10, tree, &arena);
    autocompletions_add("rm t.txt", 9, tree, &arena);
    autocompletions_add("ss", 3, tree, &arena);

    struct Autocompletion_Node* search_result =
        autocompletions_search_string((struct eskilib_String){.value = "ls | ", .length = 6}, tree);
    eskilib_assert(search_result != NULL);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = autocompletions_get("ls | ", 6, autocomplete, tree, scratch_arena);

    eskilib_assert(match_count == 3);
    eskilib_assert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void autocompletions_matches_no_results_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", 3, tree, &arena);
    autocompletions_add("ls | wc -c", 11, tree, &arena);
    autocompletions_add("ls | sort", 10, tree, &arena);
    autocompletions_add("ls | sort | wc -c", 18, tree, &arena);
    autocompletions_add("ls > t.txt", 11, tree, &arena);
    autocompletions_add("cat t.txt", 10, tree, &arena);
    autocompletions_add("rm t.txt", 9, tree, &arena);
    autocompletions_add("ss", 3, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = autocompletions_get("n", 2, autocomplete, tree, scratch_arena);

    eskilib_assert(match_count == 0);
    eskilib_assert(autocomplete[0].value == NULL);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void autocompletions_matches_multiple_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", 3, tree, &arena);
    autocompletions_add("ls | wc -c", 11, tree, &arena);
    autocompletions_add("ls | sort", 10, tree, &arena);
    autocompletions_add("ls | sort | wc -c", 18, tree, &arena);
    autocompletions_add("ls > t.txt", 11, tree, &arena);
    autocompletions_add("cat t.txt", 10, tree, &arena);
    autocompletions_add("rm t.txt", 9, tree, &arena);
    autocompletions_add("ss", 3, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = autocompletions_get("ls | ", 6, autocomplete, tree, scratch_arena);

    eskilib_assert(match_count == 3);
    eskilib_assert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    struct Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    match_count = autocompletions_get("l", 2, autocomplete_two, tree, scratch_arena);

    eskilib_assert(match_count == 5);
    eskilib_assert(memcmp(autocomplete_two[0].value, "s", 2) == 0);
    eskilib_assert(memcmp(autocomplete_two[1].value, "s > t.txt", 10) == 0);
    eskilib_assert(memcmp(autocomplete_two[2].value, "s | sort", 9) == 0);
    eskilib_assert(memcmp(autocomplete_two[3].value, "s | sort | wc -c", 17) == 0);
    eskilib_assert(memcmp(autocomplete_two[4].value, "s | wc -c", 10) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void autocompletions_matches_multiple_simulation_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = autocompletions_alloc(&arena);
    eskilib_assert(tree != NULL);

    autocompletions_add("ls", 3, tree, &arena);
    autocompletions_add("ls | wc -c", 11, tree, &arena);
    autocompletions_add("ls | sort", 10, tree, &arena);
    autocompletions_add("ls | sort | wc -c", 18, tree, &arena);
    autocompletions_add("ls > t.txt", 11, tree, &arena);
    autocompletions_add("cat t.txt", 10, tree, &arena);
    autocompletions_add("rm t.txt", 9, tree, &arena);
    autocompletions_add("ss", 3, tree, &arena);
    autocompletions_add("nvim", 5, tree, &arena);
    autocompletions_add("nvim .", 7, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    uint_fast32_t match_count = autocompletions_get("l", 2, autocomplete, tree, scratch_arena);

    eskilib_assert(match_count == 5);
    eskilib_assert(memcmp(autocomplete[0].value, "s", 2) == 0);
    eskilib_assert(memcmp(autocomplete[1].value, "s > t.txt", 10) == 0);
    eskilib_assert(memcmp(autocomplete[2].value, "s | sort", 9) == 0);
    eskilib_assert(memcmp(autocomplete[3].value, "s | sort | wc -c", 17) == 0);
    eskilib_assert(memcmp(autocomplete[4].value, "s | wc -c", 10) == 0);

    struct Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};

    match_count = autocompletions_get("ls", 3, autocomplete_two, tree, scratch_arena);

    eskilib_assert(match_count == 4);
    eskilib_assert(memcmp(autocomplete_two[0].value, " > t.txt", 9) == 0);
    eskilib_assert(memcmp(autocomplete_two[1].value, " | sort", 8) == 0);
    eskilib_assert(memcmp(autocomplete_two[2].value, " | sort | wc -c", 16) == 0);
    eskilib_assert(memcmp(autocomplete_two[3].value, " | wc -c", 8) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void autocompletions_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(autocompletions_add_length_mismatch_test);
    eskilib_test_run(autocompletions_add_test);
    eskilib_test_run(autocompletions_add_duplicate_test);
    eskilib_test_run(autocompletions_add_multiple_unrelated_test);
    eskilib_test_run(autocompletions_add_multiple_related_test);
    eskilib_test_run(autocompletions_search_test);
    eskilib_test_run(autocompletions_search_no_results_test);
    eskilib_test_run(autocompletions_search_commands_test);
    eskilib_test_run(autocompletions_matches_test);
    eskilib_test_run(autocompletions_matches_no_results_test);
    eskilib_test_run(autocompletions_matches_multiple_test);
    eskilib_test_run(autocompletions_matches_multiple_simulation_test);

    eskilib_test_finish();
}

int main(void)
{
    autocompletions_tests();

    return EXIT_SUCCESS;
}
