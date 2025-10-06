#pragma once

#include <stdint.h>

/* enum Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum Ops : uint8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Constant values
    OP_CONST,
    OP_NUM,
    // Shell operators
    OP_PIPE,                                  // |
    OP_STDOUT_REDIR,                          // >
    OP_STDOUT_REDIR_APPEND,                   // >>
    OP_STDIN_REDIR,                           // <
    OP_STDIN_REDIR_APPEND,                    // <<
    OP_STDERR_REDIR,                          // 2>
    OP_STDERR_REDIR_APPEND,                   // 2>>
    OP_STDOUT_AND_STDERR_REDIR,               // &>
    OP_STDOUT_AND_STDERR_REDIR_APPEND,        // &>>
    OP_BG_JOB,                                // &
    OP_AND,                                   // &&
    OP_OR,                                    // ||
    // Math (only implemented in parser)
    OP_ADD,                                   // +
    OP_SUB,                                   // -
    OP_MUL,                                   // *
    OP_DIV,                                   // /
    OP_MOD,                                   // %
    OP_EXP,                                   // **
    OP_MATH_EXPR_START,                       // $( or $((
    OP_MATH_EXPR_END,                         // ) or ))
    // Variables
    OP_VARIABLE,                              // a variable value (starting with $, $VAR)
    OP_ASSIGNMENT,                            // assigning a variable (var=val)
    // Boolean
    OP_TRUE,                                  // true
    OP_FALSE,                                 // false
    // Expansion
    OP_HOME_EXPANSION,                        // ~
    OP_GLOB_EXPANSION,                        // * or ?
    // Control flow structures
    OP_CONDITION_START,                       // [, [[
    OP_CONDITION_END,                         // ], ]]
    OP_IF,                                    // if
    OP_ELSE,                                  // else
    OP_ELIF,                                  // elif
    OP_THEN,                                  // then
    OP_FI,                                    // fi
    // Equality comparisons
    OP_EQUALS,                                // -eq
    OP_LESS_THAN,                             // -lt
    OP_LESS_THAN_OR_EQUALS,                   // -le
    OP_GREATER_THAN,                          // -gt
    OP_GREATER_THAN_OR_EQUALS,                // -ge
    // Loops
    OP_WHILE,                                 // while
    OP_FOR,                                   // for
    OP_DO,                                    // do
    OP_DONE                                   // done
};
