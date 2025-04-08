#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/arena_test_helper.h"
#include "../src/readline/autocompletions.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;

    struct Autocompletion_Node* autocompletions = autocompletions_alloc(&arena);
    autocompletions_add((char*)Data, Size, autocompletions, &arena);

    ARENA_TEST_TEARDOWN;
    return 0;
}
