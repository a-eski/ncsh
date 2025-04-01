#include <stdlib.h>
#include <string.h>

#include "../src/eskilib/eskilib_string.h"
#include "../src/eskilib/eskilib_test.h"

void eskilib_string_compare_no_length_test(void)
{
    char* val = "";
    constexpr size_t len = 0;
    bool result = eskilib_string_compare(val, len, val, len);
    eskilib_assert(!result);
}

void eskilib_string_compare_null_test(void)
{
    char* val = NULL;
    constexpr size_t len = 0;
    bool result = eskilib_string_compare(val, len, val, len);
    eskilib_assert(!result);
}

void eskilib_string_compare_true_test(void)
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = eskilib_string_compare(val, len, val, len);
    eskilib_assert(result);
}

void eskilib_string_compare_false_test(void)
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = eskilib_string_compare(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_compare_mismatched_lengths_false_test(void)
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = eskilib_string_compare(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_compare_partial_comparison_true_test(void)
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = eskilib_string_compare(val, len, val, len);
    eskilib_assert(result);
}

void eskilib_string_compare_partial_comparison_false_test(void)
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello") - 2;
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there") - 2;

    bool result = eskilib_string_compare(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_compare_const_no_length_test(void)
{
    const char* val = "";
    constexpr size_t len = 0;
    bool result = eskilib_string_compare_const(val, len, val, len);
    eskilib_assert(!result);
}

void eskilib_string_compare_const_null_test(void)
{
    const char* val = NULL;
    constexpr size_t len = 0;
    bool result = eskilib_string_compare_const(val, len, val, len);
    eskilib_assert(!result);
}

void eskilib_string_compare_const_true_test(void)
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello") - 1;
    bool result = eskilib_string_compare_const(val, len, val, len);
    eskilib_assert(result);
}

void eskilib_string_compare_const_false_test(void)
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = eskilib_string_compare_const(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_compare_const_mismatched_lengths_false_test(void)
{
    char* val = "hello";
    constexpr size_t len = sizeof("hello");
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there");

    bool result = eskilib_string_compare_const(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_compare_const_partial_comparison_true_test(void)
{
    char* val = "hello";
    // only compare the first three characters, 'hel'
    constexpr size_t len = sizeof("hello") - 1 - 2;
    bool result = eskilib_string_compare_const(val, len, val, len);
    eskilib_assert(result);
}

void eskilib_string_compare_const_partial_comparison_false_test(void)
{
    char* val = "hello hello";
    constexpr size_t len = sizeof("hello hello") - 1 - 2;
    char* val_two = "hello there";
    constexpr size_t len_two = sizeof("hello there") - 1 - 2;

    bool result = eskilib_string_compare_const(val, len, val_two, len_two);

    eskilib_assert(!result);
}

void eskilib_string_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(eskilib_string_compare_no_length_test);
    eskilib_test_run(eskilib_string_compare_null_test);
    eskilib_test_run(eskilib_string_compare_true_test);
    eskilib_test_run(eskilib_string_compare_false_test);
    eskilib_test_run(eskilib_string_compare_mismatched_lengths_false_test);
    eskilib_test_run(eskilib_string_compare_partial_comparison_true_test);
    eskilib_test_run(eskilib_string_compare_partial_comparison_false_test);
    eskilib_test_run(eskilib_string_compare_const_no_length_test);
    eskilib_test_run(eskilib_string_compare_const_null_test);
    eskilib_test_run(eskilib_string_compare_const_true_test);
    eskilib_test_run(eskilib_string_compare_const_false_test);
    eskilib_test_run(eskilib_string_compare_const_mismatched_lengths_false_test);
    eskilib_test_run(eskilib_string_compare_const_partial_comparison_true_test);
    eskilib_test_run(eskilib_string_compare_const_partial_comparison_false_test);

    eskilib_test_finish();
}

int main(void)
{
    eskilib_string_tests();

    return EXIT_SUCCESS;
}
