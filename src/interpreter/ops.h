#pragma once

#include <stdint.h>

/* enum Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum Ops : uint8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Constant value
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
    OP_MATH_EXPRESSION_START = 20, // $(
    OP_MATH_EXPRESSION_END = 21,   // )
    // Variables
    OP_VARIABLE = 22,   // a variable value (starting with $, $VAR)
    OP_ASSIGNMENT = 23, // assigning a variable (var=val)
    // Boolean
    OP_TRUE = 24,  // true
    OP_FALSE = 25, // false
    // Expansion
    OP_HOME_EXPANSION = 26, // ~
    OP_GLOB_EXPANSION = 27, // * or ?
    // Control flow structures
    OP_CONDITION_START = 28,
    OP_CONDITION_END = 29,
    OP_IF = 30,
    OP_ELSE = 31,
    OP_ELIF = 32,
    OP_THEN = 33,
    OP_FI = 34,
    // Equality comparisons
    OP_EQUALS = 35,
    OP_LESS_THAN = 36,
    OP_GREATER_THAN = 37,
};
