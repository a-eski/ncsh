// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_H_
#define NCSH_H_

#include <stdint.h>

/* ncsh
 * Main interactive loop of the shell.
 * Returns: exit status, see ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh(void);

#endif // !NCSH_H_
