/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena.h"
#include "eskilib/eresult.h"
#include "var.h"

#define PARSER_TOKENS_LIMIT 128

/* enum Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum Ops : uint_fast8_t {
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
    // Variables
    OP_VARIABLE = 22,
    OP_ASSIGNMENT = 23,
};

/* struct Args
 * The output of the parser.
 * Contains bytecodes, strings, and the string's lengths as well as a count
 * Gets sent to the VM as input to the VM. */
struct Args {
    size_t count;      // Number of lengths/values/ops
    size_t* lengths;   // Length of the constants stored in values
    uint_fast8_t* ops; // Ops: The bytecode
    char** values;     // Constant values needed to be referenced by the VM
    struct var vars;   // Variables: values of variables that can be looked up by key
};

/* parser_init
 * Allocate memory for the parser that lives for the lifetime of the shell.
 * Returns: enum eresult, E_SUCCESS is successful, E_FAILURE_NULL_REFERENCE if *args is null
 */
enum eresult parser_init(struct Args* const restrict args, struct Arena* const arena);

/* parser_parse
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 */
void parser_parse(const char* const restrict line, const size_t length, struct Args* const restrict args,
                  struct Arena* arena, struct Arena* scratch_arena);

/* parser_parse_noninteractive
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Used for noninteractive mode.
 */
void parser_parse_noninteractive(const char** const restrict inputs, const size_t inputs_count,
                                 struct Args* const restrict args, struct Arena* arena, struct Arena* scratch_arena);
