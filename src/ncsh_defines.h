// Copyright (c) ncsh by Alex Eski 2024

// defines used in multiple places
#ifndef NCSH_DEFINES_H_
#define NCSH_DEFINES_H_

#include "ncsh_configurables.h"

#ifdef NCSH_DEBUG
#include "ncsh_debug.h"
#endif /* ifdef NCSH_DEBUG */

#define NCSH_ERROR_STDOUT "ncsh: Error writing to stdout"
#define NCSH_ERROR_STDIN "ncsh: Error writing to stdin"

#define EXIT_IO_FAILURE -1
// #define EXIT_SUCCESS 0 // From stdlib.h
// #define EXIT_FAILURE 1 // From stdlib.h
#define EXIT_SUCCESS_END 2
#define EXIT_SUCCESS_EXECUTE 3
#define EXIT_CONTINUE 4

#define ncsh_write(str, len)                                                                                           \
    if (write(STDOUT_FILENO, str, len) == -1) {                                                                        \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        fflush(stderr);                                                                                                \
        return EXIT_FAILURE;                                                                                           \
    }

#define ncsh_write_literal(str)                                                                                        \
    if (write(STDOUT_FILENO, str, sizeof(str) - 1) == -1) {                                                            \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        fflush(stderr);                                                                                                \
        return EXIT_FAILURE;                                                                                           \
    }

// values returned from executing commands
#define NCSH_COMMAND_NONE -5 // command not run yet
#define NCSH_COMMAND_EXECVP_FAILED -3
#define NCSH_COMMAND_SYNTAX_ERROR -2
#define NCSH_COMMAND_EXIT_FAILURE -1
#define NCSH_COMMAND_EXIT 0
#define NCSH_COMMAND_SUCCESS_CONTINUE 1
#define NCSH_COMMAND_FAILED_CONTINUE 2

#define NCSH_MAX_AUTOCOMPLETION_MATCHES 32 // max number of matches for autocompletion
#define NCSH_MAX_INPUT 1024                // max input for reading in a line

#endif // !NCSH_DEFINES_H_
