// Copyright (c) ncsh by Alex Eski 2024

// defines used in multiple places
#ifndef ncsh_defines_h
#define ncsh_defines_h

// #define NCSH_DEBUG
#ifdef NCSH_DEBUG
#include "ncsh_debug.h"
#endif /* ifdef NCSH_DEBUG */

// Configurable: use entire directory path (0) or use short directory (1) for prompt line
#define NCSH_SHORT_DIRECTORY 1

#define NCSH_ERROR_STDOUT "ncsh: Error writing to stdout"
#define NCSH_ERROR_STDIN "ncsh: Error writing to stdin"

// values returned from main
#define NCSH_EXIT_SUCCESS 0
#define NCSH_EXIT_FAILURE -1

// values returned from executing commands
#define NCSH_COMMAND_EXIT_FAILURE -1
#define NCSH_COMMAND_EXIT 0
#define NCSH_COMMAND_CONTINUE 1

#define NCSH_MAX_AUTOCOMPLETION_MATCHES 32 // max number of matches for autocompletion
#define NCSH_MAX_INPUT 528 // max input for reading in a line

#endif // !ncsh_defines_h

