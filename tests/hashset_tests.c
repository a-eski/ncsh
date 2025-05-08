#include <stddef.h>

#include "../src/eskilib/str.h"
#include "../src/eskilib/etest.h"
#include "../src/readline/hashset.h"
#include "lib/arena_test_helper.h"

void hashset_malloc_default_size_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Hashset hset;
    hashset_malloc(0, &scratch_arena, &hset);

    eassert(hset.capacity > 0);
    eassert(hset.size == 0);
    eassert(hset.entries != NULL);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void hashset_add_one_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct Hashset hmap;
    hashset_malloc(0, &scratch_arena, &hmap);

    char* key = "world";
    struct Str val = {.value = key, .length = strlen(key) + 1};
    const char* res = hashset_set(val, &scratch_arena, &hmap);

    eassert(res);
    eassert(!memcmp((char*)res, (char*)key, 6));
    eassert(hmap.capacity);
    eassert(hmap.size == 1);

    bool exists = hashset_exists(val.value, &hmap);
    eassert(exists);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void hashset_add_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct Hashset hset;
    hashset_malloc(0, &scratch_arena, &hset);

    char* key1 = "hello";
    struct Str val1 = {.value = key1, .length = strlen(key1) + 1};
    char* key2 = "test";
    struct Str val2 = {.value = key2, .length = strlen(key2) + 1};

    const char* res1 = hashset_set(val1, &scratch_arena, &hset);
    const char* res2 = hashset_set(val2, &scratch_arena, &hset);

    eassert(hset.capacity);
    eassert(hset.size == 2);

    eassert(res1);
    eassert(!memcmp((char*)res1, (char*)key1, 6));

    eassert(res2);
    eassert(!memcmp((char*)res2, (char*)key2, 5));

    bool exists = hashset_exists(val1.value, &hset);
    eassert(exists);

    exists = hashset_exists(val2.value, &hset);
    eassert(exists);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void hashset_add_duplicate_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct Hashset hset;
    hashset_malloc(0, &scratch_arena, &hset);

    char* key = "hello";
    struct Str val = {.value = key, .length = strlen(key) + 1};
    const char* res = hashset_set(val, &scratch_arena, &hset);
    hashset_set(val, &scratch_arena, &hset);

    eassert(!memcmp((char*)res, (char*)key, 6));
    eassert(hset.size == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(hashset_malloc_default_size_test);
    etest_run(hashset_add_one_test);
    etest_run(hashset_add_multiple_test);
    etest_run(hashset_add_duplicate_test);

    etest_finish();

    return EXIT_SUCCESS;
}
