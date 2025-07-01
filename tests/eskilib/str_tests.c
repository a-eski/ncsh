#include <stdlib.h>
#include <string.h>

#include "../../src/eskilib/etest.h"
#include "../../src/eskilib/str.h"

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

int main()
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

    etest_finish();

    return EXIT_SUCCESS;
}
