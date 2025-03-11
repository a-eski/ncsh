// Copyright (c) ncsh by Alex Eski 2025

#include <unistd.h>

#include "ncsh.h"
#include "ncsh_noninteractive.h"

/* main
 * Entry point.
 * Controls whether the shell runs in interactive or noninteractive mode.
 * Returns: exit result, see values in ncsh_defines.h (EXIT_*)
 */
int main(int argc, char** argv)
{
    if (argc > 1 || !isatty(STDIN_FILENO)) {
        return (int)ncsh_noninteractive(argc, (const char** const)argv);
    }
    else {
        return (int)ncsh();
    }
}
