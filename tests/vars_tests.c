#include <stddef.h>
#include <stdlib.h>

#include "../src/eskilib/etest.h"
#include "../src/interpreter/vars.h"
#include "lib/arena_test_helper.h"

void vars_malloc_default_size_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Vars vars = {0};
    vars_malloc(&scratch_arena, &vars);

    eassert(vars.capacity > 0);
    eassert(vars.size == 0);
    eassert(vars.entries != NULL);
    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vars_add_one_test()
{
    SCRATCH_ARENA_TEST_SETUP;

    Vars vars = {0};
    vars_malloc(&scratch_arena, &vars);

    char* key = "hello";
    Str val = {.value = "world", .length = 6};
    const char* res = vars_set(key, &val, &scratch_arena, &vars);

    eassert(res);
    eassert(!memcmp(res, key, 6));
    eassert(vars.capacity > 0);
    eassert(vars.size == 1);

    Str* get_res = vars_get(key, &vars);
    eassert(get_res->value);
    eassert(get_res->length == val.length);
    eassert(!memcmp(val.value, get_res->value, val.length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vars_add_multiple_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Vars vars = {0};
    vars_malloc(&scratch_arena, &vars);

    char* key1 = "hello";
    Str val1 = {.value = "world", .length = 6};
    char* key2 = "test";
    Str val2 = {.value = "success", .length = 8};

    const char* res1 = vars_set(key1, &val1, &scratch_arena, &vars);
    const char* res2 = vars_set(key2, &val2, &scratch_arena, &vars);

    eassert(vars.capacity > 0);
    eassert(vars.size == 2);

    eassert(res1);
    eassert(!memcmp(res1, key1, 6));

    eassert(res2);
    eassert(!memcmp(res2, key2, 5));

    Str* get_res1 = vars_get(key1, &vars);
    eassert(get_res1->value);
    eassert(get_res1->length == val1.length);
    eassert(!memcmp(get_res1->value, val1.value, get_res1->length));

    Str* get_res2 = vars_get(key2, &vars);
    eassert(get_res2->value);
    eassert(get_res2->length = val2.length);
    eassert(!memcmp(get_res2->value, val2.value, get_res2->length));

    SCRATCH_ARENA_TEST_TEARDOWN;
}

void vars_add_duplicate_test()
{
    SCRATCH_ARENA_TEST_SETUP;
    Vars vars = {0};
    vars_malloc(&scratch_arena, &vars);

    char* key = "hello";
    Str val = {.value = "world", .length = 6};
    const char* res = vars_set(key, &val, &scratch_arena, &vars);
    eassert(res);
    vars_set(key, &val, &scratch_arena, &vars);

    eassert(!memcmp(res, key, 6));
    eassert(vars.size == 1);

    SCRATCH_ARENA_TEST_TEARDOWN;
}

int main()
{
    etest_start();

    etest_run(vars_malloc_default_size_test);
    etest_run(vars_add_one_test);
    etest_run(vars_add_multiple_test);
    etest_run(vars_add_duplicate_test);

    etest_finish();

    return EXIT_SUCCESS;
}
