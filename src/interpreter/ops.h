#pragma once

#include <stdint.h>

/* enum Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum Ops : uint8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Constant value
    OP_CONST = 1,
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
    OP_TRUE,  // true
    OP_FALSE, // false
    // Expansion
    OP_HOME_EXPANSION, // ~
    OP_GLOB_EXPANSION, // * or ?
    // Control flow structures
    OP_CONDITION_START,
    OP_CONDITION_END,
    OP_IF,
    OP_ELSE,
    OP_ELIF,
    OP_THEN,
    OP_FI,
    // Equality comparisons
    OP_EQUALS,
    OP_LESS_THAN,
    OP_LESS_THAN_OR_EQUALS,
    OP_GREATER_THAN,
    OP_GREATER_THAN_OR_EQUALS,
    // Loops
    OP_WHILE,
    OP_FOR,
    OP_DO,
    OP_DONE
};
