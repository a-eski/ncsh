#include <stdlib.h>
#include <string.h>

#include "../../src/defines.h" // used for macro NCSH_MAX_AUTOCOMPLETION_MATCHES
#include "../../src/eskilib/etest.h"
#include "../../src/readline/ac.h"
#include "../lib/arena_test_helper.h"

void ac_add_length_mismatch_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string = {.value = "and", .length = 3};
    ac_add(string.value, string.length, tree, &arena);

    // not crashing is a test pass here

    ARENA_TEST_TEARDOWN;
}

void ac_add_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string = {.value = "and", .length = 4};
    ac_add(string.value, string.length, tree, &arena);

    // sanity check: unrelated letters are null
    eassert(tree->nodes[char_to_index('b')] == NULL);
    eassert(tree->nodes[char_to_index('c')] == NULL);
    eassert(tree->nodes[char_to_index('d')] == NULL);
    eassert(tree->nodes[char_to_index('e')] == NULL);

    Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);
    eassert(first_node->nodes[char_to_index('b')] == NULL); // sanity check
    Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);
    eassert(second_node->nodes[char_to_index('b')] == NULL); // sanity check
    Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == true);
    eassert(third_node->nodes[char_to_index('b')] == NULL); // sanity check

    ARENA_TEST_TEARDOWN;
}

void ac_add_spaces_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string = {.value = "ls | wc -c", .length = 11};
    ac_add(string.value, string.length, tree, &arena);

    Autocompletion_Node* first_node = tree->nodes[char_to_index('l')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);
    Autocompletion_Node* second_node = first_node->nodes[char_to_index('s')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);
    Autocompletion_Node* third_node = second_node->nodes[char_to_index(' ')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == false);
    Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('|')];
    eassert(fourth_node != NULL);
    eassert(fourth_node->is_end_of_a_word == false);
    Autocompletion_Node* fifth_node = fourth_node->nodes[char_to_index(' ')];
    eassert(fifth_node != NULL);
    eassert(fifth_node->is_end_of_a_word == false);
    Autocompletion_Node* sixth_node = fifth_node->nodes[char_to_index('w')];
    eassert(sixth_node != NULL);
    eassert(sixth_node->is_end_of_a_word == false);
    Autocompletion_Node* seventh_node = sixth_node->nodes[char_to_index('c')];
    eassert(seventh_node != NULL);
    eassert(seventh_node->is_end_of_a_word == false);
    Autocompletion_Node* eighth_node = seventh_node->nodes[char_to_index(' ')];
    eassert(eighth_node != NULL);
    eassert(eighth_node->is_end_of_a_word == false);
    Autocompletion_Node* ninth_node = eighth_node->nodes[char_to_index('-')];
    eassert(ninth_node != NULL);
    eassert(ninth_node->is_end_of_a_word == false);
    Autocompletion_Node* tenth_node = ninth_node->nodes[char_to_index('c')];
    eassert(tenth_node != NULL);
    eassert(tenth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_add_duplicate_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string = {.value = "and", .length = 4};
    ac_add(string.value, string.length, tree, &arena);
    ac_add(string.value, string.length, tree, &arena);

    Autocompletion_Node* first_node = tree->nodes[char_to_index('a')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    Autocompletion_Node* second_node = first_node->nodes[char_to_index('n')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    Autocompletion_Node* third_node = second_node->nodes[char_to_index('d')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == true);
    eassert(third_node->weight == 3); // starts at 1, each add adds 1

    ARENA_TEST_TEARDOWN;
}

void ac_add_multiple_unrelated_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string_one = {.value = "ls", .length = 3};
    ac_add(string_one.value, string_one.length, tree, &arena);
    Str string_two = {.value = "echo", .length = 5};
    ac_add(string_two.value, string_two.length, tree, &arena);

    Autocompletion_Node* ls_first_node = tree->nodes[char_to_index('l')];
    eassert(ls_first_node != NULL);
    eassert(ls_first_node->is_end_of_a_word == false);
    Autocompletion_Node* ls_second_node = ls_first_node->nodes[char_to_index('s')];
    eassert(ls_second_node != NULL);
    eassert(ls_second_node->is_end_of_a_word == true);

    Autocompletion_Node* first_node = tree->nodes[char_to_index('e')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    Autocompletion_Node* second_node = first_node->nodes[char_to_index('c')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    Autocompletion_Node* third_node = second_node->nodes[char_to_index('h')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == false);

    Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('o')];
    eassert(fourth_node != NULL);
    eassert(fourth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_add_multiple_related_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    Str string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    Str string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    // gene
    Autocompletion_Node* first_node = tree->nodes[char_to_index('g')];
    eassert(first_node != NULL);
    eassert(first_node->is_end_of_a_word == false);

    Autocompletion_Node* second_node = first_node->nodes[char_to_index('e')];
    eassert(second_node != NULL);
    eassert(second_node->is_end_of_a_word == false);

    Autocompletion_Node* third_node = second_node->nodes[char_to_index('n')];
    eassert(third_node != NULL);
    eassert(third_node->is_end_of_a_word == false);

    Autocompletion_Node* fourth_node = third_node->nodes[char_to_index('e')];
    eassert(fourth_node != NULL);
    eassert(fourth_node->is_end_of_a_word == true);

    // genetic
    Autocompletion_Node* genetic_fifth_node = fourth_node->nodes[char_to_index('t')];
    eassert(genetic_fifth_node != NULL);
    eassert(genetic_fifth_node->is_end_of_a_word == false);

    Autocompletion_Node* genetic_sixth_node = genetic_fifth_node->nodes[char_to_index('i')];
    eassert(genetic_sixth_node != NULL);
    eassert(genetic_sixth_node->is_end_of_a_word == false);

    Autocompletion_Node* genetic_seventh_node = genetic_sixth_node->nodes[char_to_index('c')];
    eassert(genetic_seventh_node != NULL);
    eassert(genetic_seventh_node->is_end_of_a_word == true);

    // genius
    Autocompletion_Node* genius_fourth_node = third_node->nodes[char_to_index('i')];
    eassert(genius_fourth_node != NULL);
    eassert(genius_fourth_node->is_end_of_a_word == false);

    Autocompletion_Node* genius_fifth_node = genius_fourth_node->nodes[char_to_index('u')];
    eassert(genius_fifth_node != NULL);
    eassert(genius_fifth_node->is_end_of_a_word == false);

    Autocompletion_Node* genius_sixth_node = genius_fifth_node->nodes[char_to_index('s')];
    eassert(genius_sixth_node != NULL);
    eassert(genius_sixth_node->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_find_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    Str string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    Str string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    Autocompletion_Node* result = ac_find("gen", tree);

    eassert(result != NULL);
    Autocompletion_Node* result_e = result->nodes[char_to_index('e')];
    eassert(result_e != NULL);
    eassert(result_e->is_end_of_a_word == true);

    ARENA_TEST_TEARDOWN;
}

void ac_find_commands_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    Autocompletion_Node* result = tree->nodes[char_to_index('l')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == false);

    result = result->nodes[char_to_index('s')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == true);

    result = result->nodes[char_to_index(' ')];
    eassert(result != NULL);
    eassert(result->is_end_of_a_word == false);

    Autocompletion_Node* search_res = ac_find("ls | ", tree);
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

void ac_find_no_results_test()
{
    ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    Str string_one = {.value = "gene", .length = 5};
    ac_add(string_one.value, string_one.length, tree, &arena);
    Str string_two = {.value = "genetic", .length = 8};
    ac_add(string_two.value, string_two.length, tree, &arena);
    Str string_three = {.value = "genius", .length = 7};
    ac_add(string_three.value, string_three.length, tree, &arena);

    Autocompletion_Node* result = ac_find("ls", tree);
    eassert(result == NULL);

    ARENA_TEST_TEARDOWN;
}

void ac_get_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    Autocompletion_Node* search_res = ac_find("ls | ", tree);
    eassert(search_res != NULL);

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t match_count = ac_get("ls | ", autocomplete, tree, scratch_arena);

    eassert(match_count == 3);
    eassert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eassert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eassert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_spaces_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    Autocompletion_Node* search_res = ac_find("ls", tree);
    eassert(search_res != NULL);

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t match_count = ac_get("ls", autocomplete, tree, scratch_arena);

    eassert(match_count == 4);
    eassert(!memcmp(autocomplete[0].value, " > t.txt", 9));
    eassert(!memcmp(autocomplete[1].value, " | sort", sizeof(" | sort")));
    eassert(!memcmp(autocomplete[2].value, " | sort | wc -c", sizeof(" | sort | wc -c")));
    eassert(!memcmp(autocomplete[3].value, " | wc -c", sizeof(" | wc -c")));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_no_results_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t match_count = ac_get("n", autocomplete, tree, scratch_arena);

    eassert(match_count == 0);
    eassert(autocomplete[0].value == NULL);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_get_multiple_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);
    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t match_count = ac_get("ls | ", autocomplete, tree, scratch_arena);

    eassert(match_count == 3);
    eassert(memcmp(autocomplete[0].value, "sort", 5) == 0);
    eassert(memcmp(autocomplete[1].value, "sort | wc -c", 13) == 0);
    eassert(memcmp(autocomplete[2].value, "wc -c", 6) == 0);

    Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
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

void ac_get_multiple_simulation_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
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

    Autocompletion autocomplete[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t match_count = ac_get("l", autocomplete, tree, scratch_arena);

    eassert(match_count == 5);
    eassert(memcmp(autocomplete[0].value, "s", 2) == 0);
    eassert(memcmp(autocomplete[1].value, "s > t.txt", 10) == 0);
    eassert(memcmp(autocomplete[2].value, "s | sort", 9) == 0);
    eassert(memcmp(autocomplete[3].value, "s | sort | wc -c", 17) == 0);
    eassert(memcmp(autocomplete[4].value, "s | wc -c", 10) == 0);

    Autocompletion autocomplete_two[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    match_count = ac_get("ls", autocomplete_two, tree, scratch_arena);

    eassert(match_count == 4);
    eassert(memcmp(autocomplete_two[0].value, " > t.txt", 9) == 0);
    eassert(memcmp(autocomplete_two[1].value, " | sort", 8) == 0);
    eassert(memcmp(autocomplete_two[2].value, " | sort | wc -c", 16) == 0);
    eassert(memcmp(autocomplete_two[3].value, " | wc -c", 8) == 0);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_first_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("ls", 3, tree, &arena);
    ac_add("ls | wc -c", 11, tree, &arena);
    ac_add("ls | sort", 10, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls | sort | wc -c", 18, tree, &arena);
    ac_add("ls > t.txt", 11, tree, &arena);

    Autocompletion_Node* search_res = ac_find("ls | ", tree);
    eassert(search_res != NULL);

    char match[NCSH_MAX_INPUT] = {0};
    uint8_t match_count = ac_first("ls | ", match, tree, scratch_arena);

    eassert(match_count == 1);
    eassert(!memcmp(match, "sort | wc -c", 13));

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void ac_first_no_matches_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    Autocompletion_Node* tree = ac_alloc(&arena);
    eassert(tree != NULL);

    ac_add("cat t.txt", 10, tree, &arena);
    ac_add("rm t.txt", 9, tree, &arena);
    ac_add("ss", 3, tree, &arena);
    ac_add("nvim", 5, tree, &arena);
    ac_add("nvim .", 7, tree, &arena);

    char match[NCSH_MAX_INPUT] = {0};
    uint8_t match_count = ac_first("ls", match, tree, scratch_arena);

    eassert(!match_count);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(ac_add_length_mismatch_test);
    etest_run(ac_add_test);
    etest_run(ac_add_spaces_test);
    etest_run(ac_add_duplicate_test);

    etest_run(ac_add_multiple_unrelated_test);
    etest_run(ac_add_multiple_related_test);

    etest_run(ac_find_test);
    etest_run(ac_find_no_results_test);
    etest_run(ac_find_commands_test);

    etest_run(ac_get_test);
    etest_run(ac_get_spaces_test);
    etest_run(ac_get_no_results_test);
    etest_run(ac_get_multiple_test);
    etest_run(ac_get_multiple_simulation_test);

    etest_run(ac_first_test);
    etest_run(ac_first_no_matches_test);

    etest_finish();

    return EXIT_SUCCESS;
}
