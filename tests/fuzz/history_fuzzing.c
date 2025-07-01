#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include "../../src/readline/history.h"
#include "../lib/arena_test_helper.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;

    History history = {};
    history_init(Str_Empty, &history, &arena);
    history_add((char*)Data, Size, &history, &arena);
    history_save(&history);

    ARENA_TEST_TEARDOWN;
    return 0;
}
