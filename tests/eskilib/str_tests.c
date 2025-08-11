#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/eskilib/etest.h"
#include "../../src/eskilib/str.h"
#include "../lib/arena_test_helper.h"

void estrcmp_no_length_test()
{
    char* val = "";
    bool result = estrcmp(val, 0, "", 0);
    eassert(!result);
}

void estrcmp_null_test()
{
    bool result = estrcmp(NULL, 0, NULL, 0);
    eassert(!result);
}

void estrcmp_s1_null_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = estrcmp(NULL, 0, val, len);
    eassert(!result);
}

void estrcmp_empty_string_test()
{
    Str str = Str_Empty;
    Str str2 = Str_Empty;
    bool result = estrcmp(str.value, str.length, str2.value, str2.length);
    eassert(!result);
}

void estrcmp_true_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = estrcmp(val, len, "hello", len);
    eassert(result);
}

void estrcmp_false_test()
{
    Str s1 = Str_New_Literal("hello hello");
    Str s2 = Str_New_Literal("hello there");

    bool result = estrcmp(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estrcmp_mismatched_lengths_false_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = estrcmp(val, len, val_two, len_two);

    eassert(!result);
}

void estrcmp_partial_comparison_true_test()
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = estrcmp(val, len, "hello", len);
    eassert(result);
}

void estrcmp_partial_comparison_false_test()
{
    Str s1 = Str_New("hello hello", sizeof("hello hello") - 2);
    Str s2 = Str_New("hello there", sizeof("hello there") - 2);

    bool result = estrcmp(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estrsplit_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* results = estrsplit(Str_Empty, '=', &s);

    eassert(!results);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrsplit_last_pos_splitter_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* results= estrsplit(Str_New_Literal("hello="), '=', &s);

    eassert(!results);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrsplit_valid_input_returns_strs_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* results = estrsplit(Str_New_Literal("HOME=/home/alex"), '=', &s);

    eassert(!memcmp(results[0].value, "HOME", sizeof("HOME") - 1));
    eassert(results[0].length == sizeof("HOME"));

    eassert(!memcmp(results[1].value, "/home/alex", sizeof("/home/alex") - 1));
    eassert(results[1].length == sizeof("/home/alex"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrsplit_valid_input_alternate_splitter_returns_strs_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* results = estrsplit(Str_New_Literal("HOME,/home/alex"), ',', &s);

    eassert(!memcmp(results[0].value, "HOME", sizeof("HOME") - 1));
    eassert(results[0].length == sizeof("HOME"));

    eassert(!memcmp(results[1].value, "/home/alex", sizeof("/home/alex") - 1));
    eassert(results[1].length == sizeof("/home/alex"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrjoin(&Str_Empty, &Str_Empty, '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_one_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrjoin(&Str_New_Literal("hello"), &Str_Empty, '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_other_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrjoin(&Str_Empty, &Str_New_Literal("hello"), '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_valid_values_returns_joined_str_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrjoin(&Str_New_Literal("hello"), &Str_New_Literal("world"), ' ', &s);

    eassert(result);
    eassert(!memcmp(result->value, "hello world", sizeof("hello world") - 1));
    eassert(result->length = sizeof("hello world"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrcat(&Str_Empty, &Str_Empty, &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_one_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrcat(&Str_New_Literal("hello"), &Str_Empty, &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_other_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrcat(&Str_Empty, &Str_New_Literal("hello"), &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_valid_values_returns_concatted_str_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Arena s = scratch_arena;

    Str* result = estrcat(&Str_New_Literal("hello"), &Str_New_Literal(", world"), &s);

    eassert(result);
    eassert(!memcmp(result->value, "hello, world", sizeof("hello, world") - 1));
    eassert(result->length = sizeof("hello, world"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void str_tests()
{
    etest_start();

    etest_run(estrcmp_no_length_test);
    etest_run(estrcmp_null_test);
    etest_run(estrcmp_s1_null_test);
    etest_run(estrcmp_empty_string_test);
    etest_run(estrcmp_true_test);
    etest_run(estrcmp_false_test);
    etest_run(estrcmp_mismatched_lengths_false_test);
    etest_run(estrcmp_partial_comparison_true_test);
    etest_run(estrcmp_partial_comparison_false_test);

    etest_run(estrsplit_bad_value_returns_null_test);
    etest_run(estrsplit_last_pos_splitter_returns_null_test);
    etest_run(estrsplit_valid_input_returns_strs_test);
    etest_run(estrsplit_valid_input_alternate_splitter_returns_strs_test);

    etest_run(estrjoin_bad_value_returns_null_test);
    etest_run(estrjoin_one_bad_value_returns_null_test);
    etest_run(estrjoin_other_bad_value_returns_null_test);
    etest_run(estrjoin_valid_values_returns_joined_str_test);

    etest_run(estrcat_bad_value_returns_null_test);
    etest_run(estrcat_one_bad_value_returns_null_test);
    etest_run(estrcat_other_bad_value_returns_null_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    str_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
