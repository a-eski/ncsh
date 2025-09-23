#include <stddef.h>
#include <stdlib.h>

#include "../../src/interpreter/lex.h"
#include "../lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    SCRATCH_ARENA_TEST_SETUP;

    Str data;
    data.length = Size + 1;
    data.value = arena_malloc(&s, Size + 1, char);
    memcpy(data.value, Data, Size);
    data.value[Size] = '\0';

    Lexemes lexemes = {0};
    lex(data, &lexemes, &scratch_arena);

    SCRATCH_ARENA_TEST_TEARDOWN;
    return EXIT_SUCCESS;
}
