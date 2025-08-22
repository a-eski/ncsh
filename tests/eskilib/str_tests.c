#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../etest.h"
#include "../../src/eskilib/str.h"
#include "../lib/arena_test_helper.h"

void estrcmp_no_length_test()
{
    char* val = "";
    bool result = estrcmp(Str_New(val, 0), Str_New("", 0));
    eassert(!result);
}

void estrcmp_a_no_length_test()
{
    char* val = "";
    bool result = estrcmp_a(val, 0, "", 0);
    eassert(!result);
}

void estrcmp_null_test()
{
    bool result = estrcmp(Str_Empty, Str_Empty);
    eassert(!result);
}

void estrcmp_a_null_test()
{
    bool result = estrcmp_a(NULL, 0, NULL, 0);
    eassert(!result);
}

void estrcmp_s1_null_test()
{
    bool result = estrcmp(Str_Empty, Str_New_Literal("hello"));
    eassert(!result);
}

void estrcmp_a_s1_null_test()
{
    bool result = estrcmp_a(NULL, 0, "hello", sizeof("hello"));
    eassert(!result);
}

void estrcmp_empty_string_test()
{
    Str str = Str_Empty;
    Str str2 = Str_Empty;
    bool result = estrcmp(str, str2);
    eassert(!result);
}

void estrcmp_a_empty_string_test()
{
    Str str = Str_Empty;
    Str str2 = Str_Empty;
    bool result = estrcmp_a(str.value, str.length, str2.value, str2.length);
    eassert(!result);
}

void estrcmp_true_test()
{
    Str val = Str_New_Literal("hello");
    bool result = estrcmp(val, Str_New_Literal("hello"));
    eassert(result);
}

void estrcmp_a_true_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = estrcmp_a(val, len, "hello", len);
    eassert(result);
}

void estrcmp_false_test()
{
    Str s1 = Str_New_Literal("hello hello");
    Str s2 = Str_New_Literal("hello there");

    bool result = estrcmp(s1, s2);

    eassert(!result);
}

void estrcmp_a_false_test()
{
    Str s1 = Str_New_Literal("hello hello");
    Str s2 = Str_New_Literal("hello there");

    bool result = estrcmp_a(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estrcmp_mismatched_lengths_false_test()
{
    bool result = estrcmp(Str_New_Literal("hello"), Str_New_Literal("hello there"));

    eassert(!result);
}

void estrcmp_a_mismatched_lengths_false_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = estrcmp_a(val, len, val_two, len_two);

    eassert(!result);
}

void estrcmp_partial_comparison_true_test()
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = estrcmp(Str_New(val, len), Str_New("hello", len));
    eassert(result);
}

void estrcmp_a_partial_comparison_true_test()
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = estrcmp_a(val, len, "hello", len);
    eassert(result);
}

void estrcmp_partial_comparison_false_test()
{
    Str s1 = Str_New("hello hello", sizeof("hello hello") - 2);
    Str s2 = Str_New("hello there", sizeof("hello there") - 2);

    bool result = estrcmp(s1, s2);

    eassert(!result);
}

void estrcmp_a_partial_comparison_false_test()
{
    Str s1 = Str_New("hello hello", sizeof("hello hello") - 2);
    Str s2 = Str_New("hello there", sizeof("hello there") - 2);

    bool result = estrcmp_a(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estrsplit_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* results = estrsplit(Str_Empty, '=', &s);

    eassert(!results);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrsplit_last_pos_splitter_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* results= estrsplit(Str_New_Literal("hello="), '=', &s);

    eassert(!results);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrsplit_valid_input_returns_strs_test()
{
    SCRATCH_ARENA_TEST_SETUP;

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

    Str* result = estrjoin(&Str_Empty, &Str_Empty, '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_one_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrjoin(&Str_New_Literal("hello"), &Str_Empty, '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_other_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrjoin(&Str_Empty, &Str_New_Literal("hello"), '=', &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrjoin_valid_values_returns_joined_str_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrjoin(&Str_New_Literal("hello"), &Str_New_Literal("world"), ' ', &s);

    eassert(result);
    eassert(!memcmp(result->value, "hello world", sizeof("hello world") - 1));
    eassert(result->length = sizeof("hello world"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrcat(&Str_Empty, &Str_Empty, &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_one_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrcat(&Str_New_Literal("hello"), &Str_Empty, &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_other_bad_value_returns_null_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrcat(&Str_Empty, &Str_New_Literal("hello"), &s);

    eassert(!result);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrcat_valid_values_returns_concatted_str_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Str* result = estrcat(&Str_New_Literal("hello"), &Str_New_Literal(", world"), &s);

    eassert(result);
    eassert(!memcmp(result->value, "hello, world", sizeof("hello, world") - 1));
    eassert(result->length = sizeof("hello, world"));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estridx_returns_idx_test()
{
    size_t idx = estridx(&Str_New_Literal("vim=nvim"), '=');

    eassert(idx == 3);
}

void estrtoarr_single_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    constexpr size_t n = 6;
    Str vals[n] = {
        [0] = Str_New_Literal("ls")
    };

    char** res = estrtoarr(vals, n, &s);

    eassert(res);
    eassert(!memcmp(res[0], vals[0].value, vals[0].length - 1));
    eassert(strlen(res[0]) == vals[0].length - 1);
    eassert(!res[1]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrtoarr_two_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    constexpr size_t n = 6;
    Str vals[n] = {
        [0] = Str_New_Literal("ls"),
        [1] = Str_New_Literal("sort")
    };

    char** res = estrtoarr(vals, n, &s);

    eassert(res);
    eassert(!memcmp(res[0], vals[0].value, vals[0].length - 1));
    eassert(strlen(res[0]) == vals[0].length - 1);
    eassert(!memcmp(res[1], vals[1].value, vals[1].length - 1));
    eassert(strlen(res[1]) == vals[1].length - 1);
    eassert(!res[2]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrtoarr_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    constexpr size_t n = 6;
    Str vals[n] = {
        [0] = Str_New_Literal("ls"),
        [1] = Str_New_Literal("sort"),
        [2] = Str_New_Literal("head"),
        [3] = Str_New_Literal("-1"),
        [4] = Str_New_Literal("wc"),
        [5] = Str_New_Literal("-c")
    };

    char** res = estrtoarr(vals, n, &s);

    eassert(res);
    eassert(!memcmp(res[0], vals[0].value, vals[0].length - 1));
    eassert(strlen(res[0]) == vals[0].length - 1);
    eassert(!memcmp(res[1], vals[1].value, vals[1].length - 1));
    eassert(strlen(res[1]) == vals[1].length - 1);
    eassert(!memcmp(res[2], vals[2].value, vals[2].length - 1));
    eassert(strlen(res[2]) == vals[2].length - 1);
    eassert(!memcmp(res[3], vals[3].value, vals[3].length - 1));
    eassert(strlen(res[3]) == vals[3].length - 1);
    eassert(!memcmp(res[4], vals[4].value, vals[4].length - 1));
    eassert(strlen(res[4]) == vals[4].length - 1);
    eassert(!memcmp(res[5], vals[5].value, vals[5].length - 1));
    eassert(strlen(res[5]) == vals[5].length - 1);
    eassert(!res[6]);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void estrtrim_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    auto str = estrdup(&Str_New_Literal("nvim  "), &s);

    estrtrim(str);

    auto expected = Str_New_Literal("nvim");
    eassert(str->length == expected.length);
    eassert(!memcmp(str->value, expected.value, str->length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);
    auto str = Str_New_Literal("ncsh");

    sb_add(&str, sb, &s);
    eassert(sb->n == 1);
    auto res = sb_to_str(sb, &s);

    eassert(res);
    eassert(res->length == str.length);
    eassert(!memcmp(res->value, str.value, str.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_many_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    sb_add(&Str_New_Literal("/home"), sb, &s);
    eassert(sb->n == 1);
    sb_add(&Str_New_Literal("/alex"), sb, &s);
    eassert(sb->n == 2);
    sb_add(&Str_New_Literal("/ncsh"), sb, &s);
    eassert(sb->n == 3);
    auto res = sb_to_str(sb, &s);

    auto expected = Str_New_Literal("/home/alex/ncsh");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_realloc_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = &Str_New_Literal("/alex");
    for (size_t i = 0; i < 15; ++i) {
        sb_add(str, sb, &s);
        eassert(sb->n = i + 1);
    }
    eassert(sb->c = SB_START_N * 2);

    auto res = sb_to_str(sb, &s);

    auto expected = Str_New_Literal("/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_multiple_realloc_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = &Str_New_Literal("/alex");
    for (size_t i = 0; i < 25; ++i) {
        sb_add(str, sb, &s);
        eassert(sb->n = i + 1);
    }
    eassert(sb->c = SB_START_N * 2 * 2);

    auto res = sb_to_str(sb, &s);

    auto expected = Str_New_Literal("/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);
    auto str = Str_New_Literal("ncsh");

    sb_add(&str, sb, &s);
    eassert(sb->n == 1);
    auto res = sb_to_joined_str(sb, ':', &s);

    eassert(res);
    eassert(res->length == str.length);
    eassert(!memcmp(res->value, str.value, str.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_many_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    sb_add(&Str_New_Literal("/home"), sb, &s);
    eassert(sb->n == 1);
    sb_add(&Str_New_Literal("alex"), sb, &s);
    eassert(sb->n == 2);
    sb_add(&Str_New_Literal("ncsh"), sb, &s);
    eassert(sb->n == 3);
    auto res = sb_to_joined_str(sb, '/', &s);

    auto expected = Str_New_Literal("/home/alex/ncsh");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_realloc_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = &Str_New_Literal("alex");
    for (size_t i = 0; i < 15; ++i) {
        sb_add(str, sb, &s);
        eassert(sb->n = i + 1);
    }
    eassert(sb->c = SB_START_N * 2);

    auto res = sb_to_joined_str(sb, '/', &s);

    auto expected = Str_New_Literal("alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_multiple_realloc_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = &Str_New_Literal("alex");
    for (size_t i = 0; i < 25; ++i) {
        sb_add(str, sb, &s);
        eassert(sb->n = i + 1);
    }
    eassert(sb->c = SB_START_N * 2 * 2);

    auto res = sb_to_joined_str(sb, '/', &s);

    auto expected = Str_New_Literal("alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex/alex");
    eassert(res);
    eassert(res->length == expected.length);
    eassert(!memcmp(res->value, expected.value, expected.length - 1));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_path_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = Str_Get(getenv("PATH"));

    sb_add(&str, sb, &s);
    sb_add(&Str_New_Literal("/opt/nvim-linux-x86_64/bin"), sb, &s);
    sb_add(&Str_New_Literal("/home/alex/.cargo/bin"), sb, &s);
    sb_add(&Str_New_Literal("/usr/local/opt/llvm/bin"), sb, &s);
    sb_add(&Str_New_Literal("/usr/local/go/bin"), sb, &s);

    auto res = sb_to_joined_str(sb, ':', &s);

    eassert(res->value);
    eassert(res->length > str.length);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void sb_joined_path_in_scratch_to_perm_test()
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;
    auto sb = sb_new(&s);

    auto str = Str_Get(getenv("PATH"));

    sb_add(&str, sb, &s);
    sb_add(&Str_New_Literal("/opt/nvim-linux-x86_64/bin"), sb, &s);
    sb_add(&Str_New_Literal("/home/alex/.cargo/bin"), sb, &s);
    sb_add(&Str_New_Literal("/usr/local/opt/llvm/bin"), sb, &s);
    sb_add(&Str_New_Literal("/usr/local/go/bin"), sb, &s);

    auto res = sb_to_joined_str(sb, ':', &a);

    eassert(res->value);
    eassert(res->length > str.length);

    SCRATCH_ARENA_TEST_TEARDOWN;
    ARENA_TEST_TEARDOWN;
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

    etest_run(estrcmp_a_no_length_test);
    etest_run(estrcmp_a_null_test);
    etest_run(estrcmp_a_s1_null_test);
    etest_run(estrcmp_a_empty_string_test);
    etest_run(estrcmp_a_true_test);
    etest_run(estrcmp_a_false_test);
    etest_run(estrcmp_a_mismatched_lengths_false_test);
    etest_run(estrcmp_a_partial_comparison_true_test);
    etest_run(estrcmp_a_partial_comparison_false_test);

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

    etest_run(estridx_returns_idx_test);

    etest_run(estrtoarr_single_test);
    etest_run(estrtoarr_two_test);
    etest_run(estrtoarr_multiple_test);

    etest_run(estrtrim_test);

    etest_run(sb_test);
    etest_run(sb_many_test);
    etest_run(sb_realloc_test);
    etest_run(sb_multiple_realloc_test);

    etest_run(sb_joined_test);
    etest_run(sb_joined_many_test);
    etest_run(sb_joined_realloc_test);
    etest_run(sb_joined_multiple_realloc_test);
    etest_run(sb_joined_path_test);
    etest_run(sb_joined_path_in_scratch_to_perm_test);

    etest_finish();
}

#ifndef TEST_ALL
int main()
{
    str_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
