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

void alias_check_alias_found_test()
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

void alias_check_alias_found_multiple_chars_test()
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

int main()
{
    ARENA_TEST_SETUP;
    test_arena = &arena;

    etest_start();

    etest_run(alias_check_no_alias_test);
    etest_run(alias_check_alias_found_test);
    etest_run(alias_check_alias_found_multiple_chars_test);

    etest_finish();

    ARENA_TEST_TEARDOWN;

    return EXIT_SUCCESS;
}
