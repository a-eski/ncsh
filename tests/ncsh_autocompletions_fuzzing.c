#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <readline/ncsh_autocompletions.h>

#include "lib/ncsh_arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    NCSH_ARENA_TEST_SETUP;

    struct ncsh_Autocompletion_Node* autocompletions = ncsh_autocompletions_alloc(&arena);
    ncsh_autocompletions_add((char*)Data, Size, autocompletions, &arena);

    NCSH_ARENA_TEST_TEARDOWN;
    return 0;
}
