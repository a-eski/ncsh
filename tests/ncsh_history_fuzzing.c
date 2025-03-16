#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/ncsh_arena_test_helper.h"
#include "../src/readline/ncsh_history.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    NCSH_ARENA_TEST_SETUP;

    struct ncsh_History history = {};
    ncsh_history_init(eskilib_String_Empty, &history, &arena);
    ncsh_history_add((char*)Data, Size, &history, &arena);
    ncsh_history_save(&history);

    NCSH_ARENA_TEST_TEARDOWN;
    return 0;
}
