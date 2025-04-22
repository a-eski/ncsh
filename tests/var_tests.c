#include "../src/eskilib/estr.h"
#include "../src/eskilib/etest.h"
#include "../src/var.h"
#include "lib/arena_test_helper.h"
#include <stddef.h>
#include <stdlib.h> // used by macros

void var_malloc_default_size_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct var vars = {0};
    var_malloc(&scratch_arena, &vars);

    eassert(vars.capacity > 0);
    eassert(vars.size == 0);
    eassert(vars.entries != NULL);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void var_add_one_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    struct var vars = {0};
    var_malloc(&scratch_arena, &vars);

    char* key = "hello";
    struct estr val = {.value = "world", .length = 6};
    const char* res = var_set(key, &val, &scratch_arena, &vars);

    eassert(res);
    eassert(!memcmp(res, key, 6));
    eassert(vars.capacity > 0);
    eassert(vars.size == 1);

    struct estr* get_res = var_get(key, &vars);
    eassert(get_res->value);
    eassert(get_res->length == val.length);
    eassert(!memcmp(val.value, get_res->value, val.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void var_add_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct var vars = {0};
    var_malloc(&scratch_arena, &vars);

    char* key1 = "hello";
    struct estr val1 = {.value = "world", .length = 6};
    char* key2 = "test";
    struct estr val2 = {.value = "success", .length = 8};

    const char* res1 = var_set(key1, &val1, &scratch_arena, &vars);
    const char* res2 = var_set(key2, &val2, &scratch_arena, &vars);

    eassert(vars.capacity > 0);
    eassert(vars.size == 2);

    eassert(res1);
    eassert(!memcmp(res1, key1, 6));

    eassert(res2);
    eassert(!memcmp(res2, key2, 5));

    struct estr* get_res1 = var_get(key1, &vars);
    eassert(get_res1->value);
    eassert(get_res1->length == val1.length);
    eassert(!memcmp(get_res1->value, val1.value, get_res1->length));

    struct estr* get_res2 = var_get(key2, &vars);
    eassert(get_res2->value);
    eassert(get_res2->length = val2.length);
    eassert(!memcmp(get_res2->value, val2.value, get_res2->length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void var_add_duplicate_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    struct var vars = {0};
    var_malloc(&scratch_arena, &vars);

    char* key = "hello";
    struct estr val = {.value = "world", .length = 6};
    const char* res = var_set(key, &val, &scratch_arena, &vars);
    eassert(res);
    var_set(key, &val, &scratch_arena, &vars);

    eassert(!memcmp(res, key, 6));
    eassert(vars.size == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(var_malloc_default_size_test);
    etest_run(var_add_one_test);
    etest_run(var_add_multiple_test);
    etest_run(var_add_duplicate_test);

    etest_finish();

    return EXIT_SUCCESS;
}
