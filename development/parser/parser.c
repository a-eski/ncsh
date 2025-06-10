#include <stdint.h>

#include "../../src/eskilib/str.h"

typedef struct Ast Ast;

struct Ast {
    enum : uint8_t {
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
        OP_CONDITION_START,
        OP_CONDITION_END,
        OP_IF,
        OP_ELSE,
        OP_THEN,
        OP_FI,
        OP_EQUALS,
        OP_LESS_THAN,
        OP_GREATER_THAN,
        OP_WHILE,
        OP_FOR,
    } type;

    union {
        // for use in cases where no value needed (gets set to 1), like shell operators and math operators, control flow
        // structures, etc.
        int op;
        // for use in cases like OP_CONSTANT, OP_VARIABLE, OP_ASSIGNMENT, OP_HOME_Expansion, etc.
        Str* constant;

    } op;
};

int main()
{
}
