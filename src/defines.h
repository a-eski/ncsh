/* Copyright ncsh by Alex Eski 2024 */

#pragma once

/* This file contains defines used in multiple places.
 * Mainly includes Macro constants, but contains some Macro functions as well
 */

#include "configurables.h" // used for macros in other files defines is included in

// clang-format off

#define NCSH_VERSION "0.0.5.3"

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

/* CMP_* Macro functions
 * compare first few characters of a string, 1, 2, or 3
 */
#define CMP_1(str1, str2) (str1[0] == str2[0])
#define CMP_2(str1, str2) (str1[0] == str2[0] && str1[1] == str2[1])
#define CMP_3(str1, str2) (str1[0] == str2[0] && str1[1] == str2[1] && str1[2] == str2[2])

/* Global failures
 * Failures the system cannot recover from so the shell terminates.
 */
#define FAILURE_SIG_HANDLER 1
#define FAILURE_SIG_HANDLER_WRITE 2
#define FAILURE_BUILTIN_WRITE 3
#define FAILURE_BUILTIN_FILE 4

// clang-format on
