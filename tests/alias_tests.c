#include <stdlib.h>

#include "../src/alias.h"
#include "etest.h"
#include "lib/arena_test_helper.h"

static Arena* ar;

void alias_check_no_alias_test()
{
    Str result = alias_check(Str_New_Literal("echo hello"));

    eassert(!result.length);
    eassert(!result.value);
}

void alias_add_then_check_alias_found_test()
{
    Str alias = Str_New_Literal("n=nvim");
    alias_add(alias, ar);

    Str result = alias_check(Str_New_Literal("n"));

    char expected_result[] = "nvim";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_new_then_check_alias_found_test()
{
    Str alias = Str_New_Literal("n");
    alias_add_new(alias, Str_New_Literal("nvim"), ar);

    Str result = alias_check(alias);

    char expected_result[] = "nvim";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_then_check_alias_found_multiple_chars_test()
{
    Str alias = Str_New_Literal("fd=fdfind");
    alias_add(alias, ar);

    Str result = alias_check(Str_New_Literal("fd"));

    char expected_result[] = "fdfind";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_new_then_check_alias_found_multiple_chars_test()
{
    Str alias = Str_New_Literal("fd");
    alias_add_new(alias, Str_New_Literal("fdfind"), ar);

    Str result = alias_check(alias);

    char expected_result[] = "fdfind";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_then_remove_test()
{
    Str alias = Str_New_Literal("n=nvim");
    alias_add(alias, ar);

    Str buffer = Str_New_Literal("nvim");
    alias_remove(buffer);

    Str result = alias_check(buffer);
    eassert(result.length == 0);
    eassert(result.value == NULL);
}

void alias_add_then_delete_test()
{
    Str alias = Str_New_Literal("n=nvim");
    alias_add(alias, ar);

    alias_delete();

    Str result = alias_check(Str_New_Literal("n"));
    eassert(result.length == 0);
    eassert(result.value == NULL);
}

void alias_tests()
{
    ARENA_TEST_SETUP;
    ar = &arena;

    etest_start();

    etest_run(alias_check_no_alias_test);
    etest_run(alias_add_then_check_alias_found_test);
    etest_run(alias_add_new_then_check_alias_found_test);
    etest_run(alias_add_then_check_alias_found_multiple_chars_test);
    etest_run(alias_add_new_then_check_alias_found_multiple_chars_test);
    etest_run(alias_add_then_remove_test);
    etest_run(alias_add_then_delete_test);

    etest_finish();

    ARENA_TEST_TEARDOWN;
}

#ifndef TEST_ALL
int main()
{
    alias_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef TEST_ALL */
