#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h> // used by macros

#include "../src/eskilib/emap.h"
#include "../src/eskilib/estr.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

void emap_malloc_default_size_test(void)
{
    SCRATCH_ARENA_TEST_SETUP;

    struct emap ht;
    bool result = emap_malloc(&scratch_arena, &ht);
    eassert(result);

    eassert(ht.capacity > 0);
    eassert(ht.size == 0);
    eassert(ht.entries != NULL);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_one_test(void)
{
    SCRATCH_ARENA_TEST_SETUP;

    struct emap ht;
    bool result = emap_malloc(&scratch_arena, &ht);
    eassert(result);

    const char* key_value = "hello";
    struct estr string = {.value = "world", .length = 6};
    const char* key = emap_set(key_value, string, &scratch_arena, &ht);

    eassert(key != NULL);
    eassert(!memcmp((char*)key, (char*)key_value, 6));
    eassert(ht.capacity > 0);
    eassert(ht.size == 1);

    struct estr result_string = emap_get(key_value, &ht);
    eassert(result_string.value != NULL);
    eassert(result_string.length == string.length);
    eassert(!memcmp(string.value, result_string.value, string.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_multiple_test(void)
{
    SCRATCH_ARENA_TEST_SETUP;
    struct emap ht;
    bool result = emap_malloc(&scratch_arena, &ht);
    eassert(result);

    const char* key_value_one = "hello";
    struct estr string_one = {.value = "world", .length = 6};
    const char* key_value_two = "test";
    struct estr string_two = {.value = "success", .length = 8};

    const char* key_one = emap_set(key_value_one, string_one, &scratch_arena, &ht);
    const char* key_two = emap_set(key_value_two, string_two, &scratch_arena, &ht);

    eassert(ht.capacity > 0);
    eassert(ht.size == 2);

    eassert(key_one != NULL);
    eassert(!memcmp((char*)key_one, (char*)key_value_one, 6));

    eassert(key_two != NULL);
    eassert(!memcmp((char*)key_two, (char*)key_value_two, 5));

    struct estr result_one = emap_get(key_value_one, &ht);
    eassert(result_one.value != NULL);
    eassert(result_one.length == string_one.length);
    eassert(!memcmp(result_one.value, string_one.value, result_one.length));

    struct estr result_two = emap_get(key_value_two, &ht);
    eassert(result_two.value != NULL);
    eassert(result_two.length = string_two.length)
        eassert(!memcmp(result_two.value, string_two.value, result_two.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_duplicate_test(void)
{
    SCRATCH_ARENA_TEST_SETUP;
    struct emap ht;
    bool result = emap_malloc(&scratch_arena, &ht);
    eassert(result);

    const char* key_value = "hello";
    struct estr string = {.value = "world", .length = 6};
    const char* key = emap_set(key_value, string, &scratch_arena, &ht);
    emap_set(key_value, string, &scratch_arena, &ht);

    eassert(!memcmp((char*)key, (char*)key_value, 6));
    eassert(ht.size == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_tests(void)
{
    etest_start();

    etest_run(emap_malloc_default_size_test);
    etest_run(emap_add_one_test);
    etest_run(emap_add_multiple_test);
    etest_run(emap_add_duplicate_test);

    etest_finish();
}

int main(void)
{
    emap_tests();

    return EXIT_SUCCESS;
}
