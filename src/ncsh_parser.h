// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_parser_h
#define ncsh_parser_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "eskilib/eskilib_result.h"

#define NCSH_PARSER_TOKENS 128

/* ncsh_Ops: bytecodes which get sent to the VM */
enum ncsh_Ops : uint_fast8_t
{
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
    // Math
    OP_ADD = 14,                                  // +
    OP_SUBTRACT = 15,                             // -
    OP_MULTIPLY = 16,                             // *
    OP_DIVIDE = 17,                               // /
    OP_MODULO = 18,                               // %
    OP_EXPONENTIATION = 19,                       // **
    OP_MATH_EXPRESSION_START = 20,                // (
    OP_MATH_EXPRESSION_END = 21,                  // )
};

// Investigate if having struct ncsh_Object would be better, where length, op code, value, and any state in one struct.
/*
struct ncsh_Object
{
    size_t length;
    uint_fast8_t op;
    char* value;
}

struct ncsh_Args
{
    uint_fast32_t count;
    struct ncsh_Object *objects;
}
*/

/* ncsh_Args: Output of the parser, contains bytecode input which gets sent to the VM */
struct ncsh_Args
{
    uint_fast32_t count;    // Number of lengths/values/ops
    size_t *lengths;        // Length of the constants stored in values
    uint_fast8_t *ops;      // ncsh_Ops: The bytecode
    char **values;          // Constant values needed to be referenced by the VM
};

bool ncsh_parser_args_is_valid(const struct ncsh_Args *args);

enum eskilib_Result ncsh_parser_args_malloc(struct ncsh_Args *args);

void ncsh_parser_args_free(struct ncsh_Args *args);

void ncsh_parser_args_free_values(struct ncsh_Args *args);

void ncsh_parser_parse(const char *line, size_t length, struct ncsh_Args *args);

#endif // !ncsh_parser_h
