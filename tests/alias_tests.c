#include <stdlib.h>

#include "../src/alias.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

Arena* test_arena;

void alias_check_no_alias_test()
{
    char buffer[] = "echo hello";
    Str result = alias_check(buffer, sizeof(buffer));

    eassert(!result.length);
    eassert(!result.value);
}

void alias_add_then_check_alias_found_test()
{
    char* alias = "n=nvim";
    size_t alias_len = strlen(alias) + 1;
    alias_add(alias, alias_len, test_arena);

    char buffer[] = "n";
    Str result = alias_check(buffer, sizeof(buffer));

    char expected_result[] = "nvim";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_new_then_check_alias_found_test()
{
    alias_add_new("n", 2, "nvim", 5, test_arena);

    char buffer[] = "n";
    Str result = alias_check(buffer, sizeof(buffer));

    char expected_result[] = "nvim";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_then_check_alias_found_multiple_chars_test()
{
    char* alias = "fd=fdfind";
    size_t alias_len = strlen(alias) + 1;
    alias_add(alias, alias_len, test_arena);

    char buffer[] = "fd";
    Str result = alias_check(buffer, sizeof(buffer));

    char expected_result[] = "fdfind";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_new_then_check_alias_found_multiple_chars_test()
{
    alias_add_new("fd", 3, "fdfind", 7, test_arena);

    char buffer[] = "fd";
    Str result = alias_check(buffer, sizeof(buffer));

    char expected_result[] = "fdfind";
    eassert(result.length == sizeof(expected_result));
    eassert(!memcmp(result.value, expected_result, sizeof(expected_result)));
}

void alias_add_then_remove_test()
{
    char* alias = "n=nvim";
    size_t alias_len = strlen(alias) + 1;
    alias_add(alias, alias_len, test_arena);

    char buffer[] = "nvim";
    alias_remove(buffer, sizeof(buffer));

    Str result = alias_check(buffer, sizeof(buffer));
    eassert(result.length == 0);
    eassert(result.value == NULL);
}

void alias_add_then_delete_test()
{
    char* alias = "n=nvim";
    size_t alias_len = strlen(alias) + 1;
    alias_add(alias, alias_len, test_arena);

    alias_delete();

    char buffer[] = "nvim";
    Str result = alias_check(buffer, sizeof(buffer));
    eassert(result.length == 0);
    eassert(result.value == NULL);
}

void alias_tests()
{
    ARENA_TEST_SETUP;
    test_arena = &arena;

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
