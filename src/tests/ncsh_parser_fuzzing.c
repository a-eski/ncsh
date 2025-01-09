#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../ncsh_parser.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    struct ncsh_Args args = {};
    ncsh_parser_args_malloc(&args);
    ncsh_parser_parse((char *)Data, Size, &args);
    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);
    return 0;
}
