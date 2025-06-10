#include <stddef.h>
#include <stdlib.h>

#include "../src/compiler/lexer.h"
#include "lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    SCRATCH_ARENA_TEST_SETUP;

    (void)lexer_lex((char*)Data, Size, &scratch_arena);

    SCRATCH_ARENA_TEST_TEARDOWN;
    return EXIT_SUCCESS;
}
