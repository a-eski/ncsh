#include <stddef.h>
#include <stdlib.h>

#include "../../src/interpreter/lexer.h"
#include "../lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    SCRATCH_ARENA_TEST_SETUP;

    Lexemes lexemes = {0};
    (void)lexer_lex((char*)Data, Size, &lexemes, &scratch_arena);

    SCRATCH_ARENA_TEST_TEARDOWN;
    return EXIT_SUCCESS;
}
