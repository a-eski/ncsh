#include <stdint.h>
#include <stdlib.h>

#include "../eskilib/eskilib_test.h"
#include "../development/draft/ncsh_arena.h"

void ncsh_arena_init_zero_test(void)
{
    eskilib_assert(!ncsh_arena_init(0));
    eskilib_assert(!ncsh_arena_init(3));
}

void ncsh_arena_init_nonzero_test(void)
{
    eskilib_assert(ncsh_arena_init(10));
    ncsh_arena_exit();
}

void ncsh_arena_malloc_bad_input_test(void)
{
    eskilib_assert(ncsh_arena_init(10));
    eskilib_assert(!ncsh_arena_malloc(0));
    eskilib_assert(!ncsh_arena_malloc(-1));
    eskilib_assert(!ncsh_arena_malloc(15));
    eskilib_assert(!ncsh_arena_malloc(SIZE_MAX));
    ncsh_arena_exit();
}

void ncsh_arena_malloc_test(void)
{
    eskilib_assert(ncsh_arena_init(10));
    eskilib_assert(ncsh_arena_malloc(5));
    eskilib_assert(ncsh_arena_malloc(5));
    ncsh_arena_exit();
}

void ncsh_arena_tests(void)
{
    eskilib_test_start();

    eskilib_test_run("ncsh_arena_init_zero_test", ncsh_arena_init_zero_test);
    eskilib_test_run("ncsh_arena_init_nonzero_test", ncsh_arena_init_nonzero_test);
    eskilib_test_run("ncsh_arena_malloc_bad_input_test", ncsh_arena_malloc_bad_input_test);
    eskilib_test_run("ncsh_arena_malloc_test", ncsh_arena_malloc_test);

    eskilib_test_finish();
}

#ifndef ncsh_TEST_ALL
int main(void)
{
    ncsh_arena_tests();

    return EXIT_SUCCESS;
}
#endif /* ifndef ncsh_TEST_ALL */
