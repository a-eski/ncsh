#include <stddef.h>
#include <stdlib.h>

#include "../src/parser.h"
#include "lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;
    SCRATCH_ARENA_TEST_SETUP;

    struct Args args = {};
    parser_init(&args, &arena);
    parser_parse((char*)Data, Size, &args, &arena, &scratch_arena);

    ARENA_TEST_TEARDOWN;
    SCRATCH_ARENA_TEST_TEARDOWN;
    return 0;
}
