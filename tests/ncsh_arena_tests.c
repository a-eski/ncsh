#include <stdlib.h>
#include <string.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/eskilib/eskilib_test.h"
#include "../src/ncsh_arena.h"

void ncsh_arena_alloc_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = alloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eskilib_assert(!memcmp(value, test_value, test_value_len));

    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_arena_multiple_alloc_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = alloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eskilib_assert(!memcmp(value, test_value, test_value_len));

    const char* test_value_two = "this is another string";
    size_t test_value_two_len = strlen(test_value_two);
    char* value_two = alloc(&arena, test_value_two_len, char);
    memcpy(value_two, test_value_two, test_value_two_len);
    eskilib_assert(!memcmp(value_two, test_value_two, test_value_two_len));

    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_arena_realloc_test(void)
{
    NCSH_ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = alloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eskilib_assert(!memcmp(value, test_value, test_value_len));

    const char* test_value_two = "this is a string with more characters";
    size_t test_value_two_len = strlen(test_value_two);
    char* realloced_value = arena_realloc(&arena, test_value_two_len, char, value, test_value_len);
    eskilib_assert(realloced_value);
    printf("%s\n", realloced_value);
    eskilib_assert(!memcmp(realloced_value, test_value, test_value_len));
    eskilib_assert(!realloced_value[test_value_len]);

    NCSH_ARENA_TEST_TEARDOWN;
}

void ncsh_arena_tests(void)
{
    eskilib_test_start();

    eskilib_test_run(ncsh_arena_alloc_test);
    eskilib_test_run(ncsh_arena_multiple_alloc_test);
    eskilib_test_run(ncsh_arena_realloc_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_arena_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
