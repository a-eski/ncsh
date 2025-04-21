#include <stddef.h>
#include <stdlib.h>

#include "../src/readline/ac.h"
#include "lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* autocompletions = ac_alloc(&arena);
    ac_add((char*)Data, Size, autocompletions, &arena);

    ARENA_TEST_TEARDOWN;
    return 0;
}
