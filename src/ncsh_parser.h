/* Copyright (c) ncsh by Alex Eski 2024 */

#ifndef NCSH_PARSER_H_
#define NCSH_PARSER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"

#define NCSH_PARSER_TOKENS 128

/* enum ncsh_Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum ncsh_Ops : uint_fast8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Constant value, need to check constants array for what it is
    OP_CONSTANT = 1,
    // Shell operators
    OP_PIPE = 2,                                  // |
    OP_STDOUT_REDIRECTION = 3,                    // >
    OP_STDOUT_REDIRECTION_APPEND = 4,             // >>
    OP_STDIN_REDIRECTION = 5,                     // <
    OP_STDIN_REDIRECTION_APPEND = 6,              // <<
    OP_STDERR_REDIRECTION = 7,                    // 2>
    OP_STDERR_REDIRECTION_APPEND = 8,             // 2>>
    OP_STDOUT_AND_STDERR_REDIRECTION = 9,         // &>
    OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10, // &>>
    OP_BACKGROUND_JOB = 11,                       // &
    OP_AND = 12,                                  // &&
    OP_OR = 13,                                   // ||
    // Math (only implemented in parser)
    OP_ADD = 14,                   // +
    OP_SUBTRACT = 15,              // -
    OP_MULTIPLY = 16,              // *
    OP_DIVIDE = 17,                // /
    OP_MODULO = 18,                // %
    OP_EXPONENTIATION = 19,        // **
    OP_MATH_EXPRESSION_START = 20, // (
    OP_MATH_EXPRESSION_END = 21,   // )
};

// TODO: Investigate if having struct ncsh_Command would be better, where length, op code, value, and any state in one struct to improve cache locality.
/*
struct ncsh_Command
{
    size_t length;
    uint_fast8_t op;
    char* value;
}

struct ncsh_Args
{
    uint_fast32_t count;
    struct ncsh_Command* commands;
}
*/

/* struct ncsh_Args
 * The output of the parser.
 * Contains bytecodes, strings, and the string's lengths as well as a count
 * Gets sent to the VM as input to the VM. */
struct ncsh_Args {
    uint_fast32_t count; // Number of lengths/values/ops
    size_t* lengths;     // Length of the constants stored in values
    uint_fast8_t* ops;   // ncsh_Ops: The bytecode
    char** values;       // Constant values needed to be referenced by the VM
};

/* ncsh_parser_args_malloc
 * Allocate memory for the parser that lives for the lifetime of the shell.
 * Returns: enum eskilib_Result, E_SUCCESS is successful.
 */
enum eskilib_Result ncsh_parser_args_malloc(struct ncsh_Args* const restrict args);

/* ncsh_parser_args_free
 * Free memory used by the parser at the end of shell's lifetime.
 */
void ncsh_parser_args_free(struct ncsh_Args* const restrict args);

/* ncsh_parser_parse
 * Parse the line into commands, command lengths, and op codes stored in struct ncsh_Args.
 * Allocates memory that is freed by ncsh_parser_free_values at the end of each main loop of the shell.
 */
void ncsh_parser_parse(const char* const restrict line,
                       const size_t length,
                       struct ncsh_Args* const restrict args);

/* ncsh_parser_parse_noninteractive
 * Parse the command line input into commands, command lengths, and op codes stored in struct ncsh_Args.
 * Allocates memory that is freed by ncsh_parser_free_values at the end of each main loop of the shell.
 */
void ncsh_parser_parse_noninteractive(const char** const restrict inputs,
                                      const size_t inputs_count,
                                      struct ncsh_Args* const restrict args);

/* ncsh_parser_args_free_values
 * Free memory used by the parser that is used during each main loop of the shell.
 * This means each main loop of the shell has values freed by this function.
 */
void ncsh_parser_args_free_values(struct ncsh_Args* const restrict args);

#endif // !NCSH_PARSER_H_
