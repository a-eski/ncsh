#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../src/defines.h"
#include "../src/eskilib/estr.h"
#include "../src/eskilib/etest.h"
#include "../src/readline/autocompletions.h"
#include "lib/arena_test_helper.h"

void ac_add_length_mismatch_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string = {.value = "and", .length = 3};
    ac_add(string.value, string.length, tree, &arena);

    // not crashing is a test pass here

    ARENA_TEST_TEARDOWN;
}

void ac_add_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string = {.value = "and", .length = 4};
    ac_add(string.value, string.length, tree, &arena);

    // sanity check: unrelated letters are null
    eassert(tree->nodes[char_to_index('b')] == NULL);
    eassert(tree->nodes[char_to_index('c')] == NULL);
    eassert(tree->nodes[char_to_index('d')] == NULL);
    eassert(tree->nodes[char_to_index('e')] == NULL);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);
    eassert(first_node->nodes[char_to_index('b')] == NULL); // sanity check
    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);
    eassert(second_node->nodes[char_to_index('b')] == NULL); // sanity check
    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == true);
    eassert(third_node->nodes[char_to_index('b')] == NULL); // sanity check

    ARENA_TEST_TEARDOWN;
}

void ac_add_duplicate_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string = {.value = "and", .length = 4};
    ac_add(string.value, string.length, tree, &arena);
    ac_add(string.value, string.length, tree, &arena);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == true);
    eassert(third_node->weight == 2);

    ARENA_TEST_TEARDOWN;
}

void ac_add_multiple_unrelated_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string_one = {.value = "ls", .length = 3};
    ac_add(string_one.value, string_one.length, tree, &arena);
    struct estr string_two = {.value = "echo", .length = 5};
    ac_add(string_two.value, string_two.length, tree, &arena);

    struct Autocompletion_Node* ls_first_node = tree->nodes[char_to_index('l')];
    eassert(ls_first_node != NULL);
    eassert(ls_first_node->is_end_of_a_word == false);
    struct Autocompletion_Node* ls_second_node = ls_first_node->nodes[char_to_index('s')];
    eassert(ls_second_node != NULL);
    eassert(ls_second_node->is_end_of_a_word == true);

    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('e')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('c')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('h')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == false);

    struct Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('o')];
    eassert(fourth_node != NULL);
    eassert(fourth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_add_multiple_related_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    struct estr string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    struct estr string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    // gene
    struct Autocompletion_Node* first_node = tree->nodes[char_to_index('g')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    struct Autocompletion_Node* second_node = first_node->nodes[char_to_index('e')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    struct Autocompletion_Node* third_node = second_node->nodes[char_to_index('n')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == false);

    struct Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('e')];
    eassert(fourth_node != NULL);
    eassert(fourth_node->is_end_of_a_word == true);

    // genetic
    struct Autocompletion_Node* genetic_fifth_node = fourth_node->nodes[char_to_index('t')];
    eassert(genetic_fifth_node != NULL);
    eassert(genetic_fifth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genetic_sixth_node = genetic_fifth_node->nodes[char_to_index('i')];
    eassert(genetic_sixth_node != NULL);
    eassert(genetic_sixth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genetic_seventh_node = genetic_sixth_node->nodes[char_to_index('c')];
    eassert(genetic_seventh_node != NULL);
    eassert(genetic_seventh_node->is_end_of_a_word == true);

    // genius
    struct Autocompletion_Node* genius_fourth_node = third_node->nodes[char_to_index('i')];
    eassert(genius_fourth_node != NULL);
    eassert(genius_fourth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genius_fifth_node = genius_fourth_node->nodes[char_to_index('u')];
    eassert(genius_fifth_node != NULL);
    eassert(genius_fifth_node->is_end_of_a_word == false);

    struct Autocompletion_Node* genius_sixth_node = genius_fifth_node->nodes[char_to_index('s')];
    eassert(genius_sixth_node != NULL);
    eassert(genius_sixth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_find_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    struct estr string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    struct estr string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    struct Autocompletion_Node* result = ac_find("gen", tree);

    eassert(result != NULL);
    struct Autocompletion_Node* result_e = result->nodes[char_to_index('e')];
    eassert(result_e != NULL);
    eassert(result_e->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_find_commands_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    struct Autocompletion_Node* result = tree->nodes[char_to_index('l')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == false);

    result = result->nodes[char_to_index('s')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == true);

    result = result->nodes[char_to_index(' ')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == false);

    struct Autocompletion_Node* search_res = ac_find("ls | ", tree);
    eassert(search_res != NULL);
    eassert(search_res->is_end_of_a_word == false);

    search_res = search_res->nodes[char_to_index('s')];
    eassert(search_res != NULL);
    eassert(search_res->is_end_of_a_word == false);

    search_res = search_res->nodes[char_to_index('o')];
    eassert(search_res != NULL);
    eassert(search_res->is_end_of_a_word == false);

    search_res = search_res->nodes[char_to_index('r')];
    eassert(search_res != NULL);
    eassert(search_res->is_end_of_a_word == false);

    search_res = search_res->nodes[char_to_index('t')];
    eassert(search_res != NULL);
    eassert(search_res->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_find_no_results_test(void)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    struct estr string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    struct estr string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    struct estr string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    struct Autocompletion_Node* result = ac_find("ls", tree);
    eassert(result == NULL);

    ARENA_TEST_TEARDOWN;
}

void ac_get_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    struct Autocompletion_Node* search_res = ac_find("ls | ", tree);
    eassert(search_res != NULL);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = ac_get("ls | ", autocomplete, tree, scratch_arena);

    eassert(match_count == 3);
    eassert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eassert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eassert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_no_results_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = ac_get("n", autocomplete, tree, scratch_arena);

    eassert(match_count == 0);
    eassert(autocomplete[0].value == NULL);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_multiple_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = ac_get("ls | ", autocomplete, tree, scratch_arena);

    eassert(match_count == 3);
    eassert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eassert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eassert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    struct Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    match_count = ac_get("l", autocomplete_two, tree, scratch_arena);

    eassert(match_count == 5);
    eassert(memcmp(autocomplete_two[0].value, "s", 2) == 0);
    eassert(memcmp(autocomplete_two[1].value, "s > t.txt", 10) == 0);
    eassert(memcmp(autocomplete_two[2].value, "s | sort", 9) == 0);
    eassert(memcmp(autocomplete_two[3].value, "s | sort | wc -c", 17) == 0);
    eassert(memcmp(autocomplete_two[4].value, "s | wc -c", 10) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_multiple_simulation_test(void)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);
    ac_add("nvim", 5, tree, &arena);
    ac_add("nvim .", 7, tree, &arena);

    struct Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint_fast32_t match_count = ac_get("l", autocomplete, tree, scratch_arena);

    eassert(match_count == 5);
    eassert(memcmp(autocomplete[0].value, "s", 2) == 0);
    eassert(memcmp(autocomplete[1].value, "s > t.txt", 10) == 0);
    eassert(memcmp(autocomplete[2].value, "s | sort", 9) == 0);
    eassert(memcmp(autocomplete[3].value, "s | sort | wc -c", 17) == 0);
    eassert(memcmp(autocomplete[4].value, "s | wc -c", 10) == 0);

    struct Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    match_count = ac_get("ls", autocomplete_two, tree, scratch_arena);

    eassert(match_count == 4);
    eassert(memcmp(autocomplete_two[0].value, " > t.txt", 9) == 0);
    eassert(memcmp(autocomplete_two[1].value, " | sort", 8) == 0);
    eassert(memcmp(autocomplete_two[2].value, " | sort | wc -c", 16) == 0);
    eassert(memcmp(autocomplete_two[3].value, " | wc -c", 8) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_tests(void)
{
    etest_start();

    etest_run(ac_add_length_mismatch_test);
    etest_run(ac_add_test);
    etest_run(ac_add_duplicate_test);
    etest_run(ac_add_multiple_unrelated_test);
    etest_run(ac_add_multiple_related_test);
    etest_run(ac_find_test);
    etest_run(ac_find_no_results_test);
    etest_run(ac_find_commands_test);
    etest_run(ac_get_test);
    etest_run(ac_get_no_results_test);
    etest_run(ac_get_multiple_test);
    etest_run(ac_get_multiple_simulation_test);

    etest_finish();
}

int main(void)
{
    ac_tests();

    return EXIT_SUCCESS;
}
