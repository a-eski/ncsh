#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/ncsh_parser.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    NCSH_ARENA_TEST_SETUP;
    NCSH_SCRATCH_ARENA_TEST_SETUP;

    struct ncsh_Args args = {};
    ncsh_parser_args_alloc(&args, &arena);
    ncsh_parser_parse((char*)Data, Size, &args, &scratch_arena);

    NCSH_ARENA_TEST_TEARDOWN;
    NCSH_SCRATCH_ARENA_TEST_TEARDOWN;
    return 0;
}
