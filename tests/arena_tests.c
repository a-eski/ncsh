#include <stdlib.h>
#include <string.h>

#include "../src/arena.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

void arena_malloc_test()
{
    ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = arena_malloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eassert(!memcmp(value, test_value, test_value_len));

    ARENA_TEST_TEARDOWN;
}

void arena_malloc_multiple_test()
{
    ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = arena_malloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eassert(!memcmp(value, test_value, test_value_len));

    const char* test_value_two = "this is another string";
    size_t test_value_two_len = strlen(test_value_two);
    char* value_two = arena_malloc(&arena, test_value_two_len, char);
    memcpy(value_two, test_value_two, test_value_two_len);
    eassert(!memcmp(value_two, test_value_two, test_value_two_len));

    ARENA_TEST_TEARDOWN;
}

void arena_realloc_test()
{
    ARENA_TEST_SETUP;

    const char* test_value = "this is a string";
    size_t test_value_len = strlen(test_value);
    char* value = arena_malloc(&arena, test_value_len, char);
    memcpy(value, test_value, test_value_len);
    eassert(!memcmp(value, test_value, test_value_len));

    const char* test_value_two = "this is a string with more characters";
    size_t test_value_two_len = strlen(test_value_two);
    char* realloced_value = arena_realloc(&arena, test_value_two_len, char, value, test_value_len);
    eassert(realloced_value);
    eassert(!memcmp(realloced_value, test_value, test_value_len));
    eassert(!realloced_value[test_value_len]);

    ARENA_TEST_TEARDOWN;
}

struct Test {
    size_t test;
};

void arena_realloc_non_char_test()
{
    ARENA_TEST_SETUP;

    const size_t initial_size = 5;
    struct Test* test_values = arena_malloc(&arena, initial_size, struct Test);
    test_values[0] = (struct Test){.test = 100};
    test_values[1] = (struct Test){.test = 200};
    test_values[2] = (struct Test){.test = 300};

    const size_t new_size = 10;
    struct Test* realloced_value = arena_realloc(&arena, new_size, struct Test, test_values, initial_size);
    eassert(realloced_value);
    eassert(realloced_value[0].test == 100);
    eassert(realloced_value[1].test == 200);
    eassert(realloced_value[2].test == 300);
    for (size_t i = 3; i < 10; ++i) {
        eassert(!realloced_value[i].test);
    }
    eassert(!memcmp(realloced_value, test_values, initial_size));

    ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(arena_malloc_test);
    etest_run(arena_malloc_multiple_test);
    etest_run(arena_realloc_test);
    etest_run(arena_realloc_non_char_test);

    etest_finish();

    return EXIT_SUCCESS;
}
