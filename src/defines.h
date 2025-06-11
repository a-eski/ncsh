/* Copyright ncsh by Alex Eski 2024 */

#pragma once

/* This file contains defines used in multiple places.
 * Mainly includes Macro constants, but contains some Macro functions as well
 */

#include "configurables.h"
#include "eskilib/ecolors.h"

// clang-format off

#include "debug.h"

#define NCSH_VERSION "0.0.3.23"

/* EXIT_* Constants
 * Exit values used by a multitude of functions and areas in the shell.
 */
#define EXIT_SYNTAX_ERROR -3
#define EXIT_FAILURE_CONTINUE -2
#define EXIT_IO_FAILURE -1
#ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0 // Defined in stdlib.h
#endif                 /* !EXIT_SUCCESS */
#ifndef EXIT_FAILURE
#    define EXIT_FAILURE 1 // Defined in stdlib.h
#endif                 /* !EXIT_FAILURE */
#define EXIT_SUCCESS_END 2
#define EXIT_SUCCESS_EXECUTE 3
#define EXIT_CONTINUE 4

/* NCSH_ERROR_*
 * Common error messages (in the amount of times they occur in code, not in use), used to give user some info like in
 * cases where shell can't read/write from stdin/stdout. */
#define NCSH_ERROR_STDOUT "ncsh: Error writing to stdout"
#define NCSH_ERROR_STDIN "ncsh: Error writing to stdin"

/* ncsh_write Macro functions
 * Common output function, uses write function to send output via stdout to the terminal.
 * ncsh_write accepts a string and its length, supports non-string literals.
 * Return: Returns nothing on success, on failure returns EXIT_FAILURE, so functions that use these need to have a
 * return value.
 */
#define ncsh_write(str, len)                                                                                           \
    if (write(STDOUT_FILENO, str, len) == -1) {                                                                        \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        fflush(stderr);                                                                                                \
        return EXIT_FAILURE;                                                                                           \
    }

/* ncsh_write_literal Macro function
 * Common output function, uses write function to send output via stdout to the terminal.
 * ncsh_write_literal accepts a string literal and uses sizeof to gets its length.
 * Return: Returns nothing on success, on failure returns EXIT_FAILURE, so functions that use need to have a return
 * value.
 */
#define ncsh_write_literal(str)                                                                                        \
    if (write(STDOUT_FILENO, str, sizeof(str) - 1) == -1) {                                                            \
        perror(RED NCSH_ERROR_STDOUT RESET);                                                                           \
        fflush(stderr);                                                                                                \
        return EXIT_FAILURE;                                                                                           \
    }

/* CMP_* Macro functions
 * compare first few characters of a string, 1, 2, or 3
 */
#define CMP_1(str1, str2) (str1[0] == str2[0])
#define CMP_2(str1, str2) (str1[0] == str2[0] && str1[1] == str2[1])
#define CMP_3(str1, str2) (str1[0] == str2[0] && str1[1] == str2[1] && str1[2] == str2[2])

#define rst restrict

// clang-format on
