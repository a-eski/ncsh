#ifndef NCSH_READLINE_H_
#define NCSH_READLINE_H_

#include <stdint.h>

#include "ncsh_types.h"

int_fast32_t ncsh_readline(struct ncsh_Input* input, struct ncsh_Shell* shell);

#endif // !NCSH_READLINE_H_
