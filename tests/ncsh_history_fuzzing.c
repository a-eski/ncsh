#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../src/readline/ncsh_history.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
    struct ncsh_History history = {};
    ncsh_history_init(eskilib_String_Empty, &history);
    assert(Size == strlen((char*)Data) + 1);
    ncsh_history_add((char*)Data, Size, &history);
    ncsh_history_exit(&history);
    return 0;
}
