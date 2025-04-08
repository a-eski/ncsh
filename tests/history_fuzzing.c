#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/arena_test_helper.h"
#include "../src/readline/history.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    ARENA_TEST_SETUP;

    struct History history = {};
    history_init(eskilib_String_Empty, &history, &arena);
    history_add((char*)Data, Size, &history, &arena);
    history_save(&history);

    ARENA_TEST_TEARDOWN;
    return 0;
}
