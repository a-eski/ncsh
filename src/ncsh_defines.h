// Copyright (c) ncsh by Alex Eski 2024

// defines used in multiple places
#ifndef ncsh_defines_h
#define ncsh_defines_h

#include "ncsh_configurables.h"

#define NCSH_ERROR_STDOUT "ncsh: Error writing to stdout"
#define NCSH_ERROR_STDIN "ncsh: Error writing to stdin"

// values returned from executing commands
#define NCSH_COMMAND_SYNTAX_ERROR -2
#define NCSH_COMMAND_EXIT_FAILURE -1
#define NCSH_COMMAND_EXIT 0
#define NCSH_COMMAND_SUCCESS_CONTINUE 1
#define NCSH_COMMAND_FAILED_CONTINUE 2

#define NCSH_MAX_AUTOCOMPLETION_MATCHES 32 // max number of matches for autocompletion
#define NCSH_MAX_INPUT 528 // max input for reading in a line

#endif // !ncsh_defines_h

