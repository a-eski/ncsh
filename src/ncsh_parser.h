/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <readline/ncsh_arena.h>

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

enum eskilib_Result ncsh_parser_args_alloc(struct ncsh_Args* const restrict args,
                                            struct ncsh_Arena* const arena);

void ncsh_parser_parse(const char* const restrict line,
                       const size_t length,
                       struct ncsh_Args* const restrict args,
                       struct ncsh_Arena* const scratch_arena);

void ncsh_parser_parse_noninteractive(const char** const restrict inputs,
                                      const size_t inputs_count,
                                      struct ncsh_Args* const restrict args,
                                      struct ncsh_Arena* const scratch_arena);
