// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_NONINTERACTIVE_H_
#define NCSH_NONINTERACTIVE_H_

#include <stdint.h>

/* ncsh_noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Returns: exit status, see ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_noninteractive(const int argc, const char** const restrict argv);

#endif // !NCSH_NONINTERACTIVE_H_
