#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/estr.h"
#include "../src/eskilib/etest.h"

void estr_compare_no_length_test()
{
    char* val = "";
    constexpr size_t len = 0;
    bool result = estrcmp(val, len, val, len);
    eassert(!result);
}

void estr_compare_null_test()
{
    char* val = NULL;
    constexpr size_t len = 0;
    bool result = estrcmp(val, len, val, len);
    eassert(!result);
}

void estr_compare_empty_string_test()
{
    struct estr str = estr_Empty;
    bool result = estrcmp(str.value, str.length, str.value, str.length);
    eassert(!result);
}

void estr_compare_true_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = estrcmp(val, len, val, len);
    eassert(result);
}

void estr_compare_false_test()
{
    const struct estr s1 = estr_New_Literal("hello hello");
    const struct estr s2 = estr_New_Literal("hello there");

    bool result = estrcmp(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estr_compare_mismatched_lengths_false_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = estrcmp(val, len, val_two, len_two);

    eassert(!result);
}

void estr_compare_partial_comparison_true_test()
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = estrcmp(val, len, val, len);
    eassert(result);
}

void estr_compare_partial_comparison_false_test()
{
    const struct estr s1 = estr_New("hello hello", sizeof("hello hello") - 2);
    const struct estr s2 = estr_New("hello there", sizeof("hello there") - 2);

    bool result = estrcmp(s1.value, s1.length, s2.value, s2.length);

    eassert(!result);
}

void estr_compare_const_no_length_test()
{
    const char* val = "";
    constexpr size_t len = 0;
    bool result = estrcmp_cc(val, len, val, len);
    eassert(!result);
}

void estr_compare_const_null_test()
{
    const char* val = NULL;
    constexpr size_t len = 0;
    bool result = estrcmp_cc(val, len, val, len);
    eassert(!result);
}

void estr_compare_const_empty_string_test()
{
    const struct estr str = estr_Empty;
    bool result = estrcmp_cc(str.value, str.length, str.value, str.length);
    eassert(!result);
}

void estr_compare_const_true_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = estrcmp_cc(val, len, val, len);
    eassert(result);
}

void estr_compare_const_false_test()
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = estrcmp_cc(val, len, val_two, len_two);

    eassert(!result);
}

void estr_compare_const_mismatched_lengths_false_test()
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = estrcmp_cc(val, len, val_two, len_two);

    eassert(!result);
}

void estr_compare_const_partial_comparison_true_test()
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = estrcmp_cc(val, len, val, len);
    eassert(result);
}

void estr_compare_const_partial_comparison_false_test()
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello") - 1 - 2;
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there") - 1 - 2;

    bool result = estrcmp_cc(val, len, val_two, len_two);

    eassert(!result);
}

int main()
{
    etest_start();

    etest_run(estr_compare_no_length_test);
    etest_run(estr_compare_null_test);
    etest_run(estr_compare_empty_string_test);
    etest_run(estr_compare_true_test);
    etest_run(estr_compare_false_test);
    etest_run(estr_compare_mismatched_lengths_false_test);
    etest_run(estr_compare_partial_comparison_true_test);
    etest_run(estr_compare_partial_comparison_false_test);
    etest_run(estr_compare_const_no_length_test);
    etest_run(estr_compare_const_null_test);
    etest_run(estr_compare_const_empty_string_test);
    etest_run(estr_compare_const_true_test);
    etest_run(estr_compare_const_false_test);
    etest_run(estr_compare_const_mismatched_lengths_false_test);
    etest_run(estr_compare_const_partial_comparison_true_test);
    etest_run(estr_compare_const_partial_comparison_false_test);

    etest_finish();

    return EXIT_SUCCESS;
}
