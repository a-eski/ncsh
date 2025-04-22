#include <stddef.h>

#include "../src/eskilib/emap.h"
#include "../src/eskilib/estr.h"
#include "../src/eskilib/etest.h"
#include "lib/arena_test_helper.h"

void emap_malloc_default_size_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct emap ht;
    emap_malloc(&scratch_arena, &ht);

    eassert(ht.capacity > 0);
    eassert(ht.size == 0);
    eassert(ht.entries != NULL);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_one_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct emap hmap;
    emap_malloc(&scratch_arena, &hmap);

    char* key = "world";
    struct estr val = {.value = key, .length = strlen(key) + 1};
    const char* res = emap_set(val, &scratch_arena, &hmap);

    eassert(res);
    eassert(!memcmp((char*)res, (char*)key, 6));
    eassert(hmap.capacity);
    eassert(hmap.size == 1);

    struct estr res_str = emap_get(val.value, &hmap);
    eassert(res_str.value);
    eassert(res_str.length == val.length);
    eassert(!memcmp(val.value, res_str.value, val.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct emap ht;
    emap_malloc(&scratch_arena, &ht);

    char* key1 = "hello";
    struct estr val1 = {.value = key1, .length = strlen(key1) + 1};
    char* key2 = "test";
    struct estr val2 = {.value = key2, .length = strlen(key2) + 1};

    const char* res1 = emap_set(val1, &scratch_arena, &ht);
    const char* res2 = emap_set(val2, &scratch_arena, &ht);

    eassert(ht.capacity);
    eassert(ht.size == 2);

    eassert(res1);
    eassert(!memcmp((char*)res1, (char*)key1, 6));

    eassert(res2);
    eassert(!memcmp((char*)res2, (char*)key2, 5));

    struct estr get_res1 = emap_get(val1.value, &ht);
    eassert(*get_res1.value);
    eassert(get_res1.length == val1.length);
    eassert(!memcmp(get_res1.value, val1.value, get_res1.length));

    struct estr get_res2 = emap_get(val2.value, &ht);
    eassert(get_res2.value);
    eassert(get_res2.length = val2.length) eassert(!memcmp(get_res2.value, val2.value, get_res2.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void emap_add_duplicate_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct emap ht;
    emap_malloc(&scratch_arena, &ht);

    char* key = "hello";
    struct estr val = {.value = key, .length = strlen(key) + 1};
    const char* res = emap_set(val, &scratch_arena, &ht);
    emap_set(val, &scratch_arena, &ht);

    eassert(!memcmp((char*)res, (char*)key, 6));
    eassert(ht.size == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(emap_malloc_default_size_test);
    etest_run(emap_add_one_test);
    etest_run(emap_add_multiple_test);
    etest_run(emap_add_duplicate_test);

    etest_finish();

    return EXIT_SUCCESS;
}
