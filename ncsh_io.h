#ifndef ncsh_io_h
#define ncsh_io_h

#include <stdint.h>

#include "ncsh_types.h"

void ncsh_write(char* string, uint_fast32_t length);

void ncsh_print_prompt(struct ncsh_Directory prompt_info);

#endif // !ncsh_io_h

