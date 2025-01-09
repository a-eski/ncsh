#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../ncsh_autocompletions.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    struct ncsh_Autocompletion_Node *autocompletions = ncsh_autocompletions_malloc();
    ncsh_autocompletions_add((char *)Data, Size, autocompletions);
    ncsh_autocompletions_free(autocompletions);
    return 0;
}
