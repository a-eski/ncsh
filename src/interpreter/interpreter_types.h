/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter_types.h: interpreter types for ncsh */

#pragma once

#include <stdint.h>
#include <unistd.h>

/****** TYPES ******/
/*** TOKEN TYPES ***/
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
    OP_CONDITION_START,
    OP_CONDITION_END,
    OP_IF,
    OP_ELSE,
    OP_ELIF,
    OP_THEN,
    OP_FI,
    OP_EQUALS,
    OP_LESS_THAN,
    OP_GREATER_THAN,
};
/* Token
 * Holds op code, length, and value of the parsed token.
 * Includes a pointer to next element in the linked list.
 */
typedef struct Token_ {
    uint8_t op; // enum Ops
    size_t len;
    char* val;
    struct Token_* next;
} Token;

/*** PREPROCSSING AND LOGIC TYPES ***/
enum Logic_Type {
    LT_NONE,
    LT_CODE,
    LT_IF,
    LT_IF_ELSE
};

typedef struct {
    size_t count;
    size_t cap;
    enum Ops* ops;
    size_t* lens;
    char** vals;
} Commands;

typedef struct {
    size_t count;
    size_t cap;
    Commands* commands;
} Statements;

union Logic_Value {
    int code;
    Token* tok;
};

typedef struct {
    enum Logic_Type type;
    union Logic_Value val;
} Logic_Result;

/* Token_Data
 * Stores information related to tokens from Args,
 * like position of redirect operations, counts of pipe commands, and file names to use to create file descriptors.
 * Output append directs output redirections to append to the file instead of writing over it.
 */
typedef struct _Token_Data {
    // Redirection
    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;

    // Background Jobs
    bool is_background_job;

    // Pipes
    uint8_t number_of_pipe_commands;

    // Control flow structures
    enum Logic_Type logic_type;
    Statements* conditions;
    Statements* if_statements;
    Statements* else_statements;
    // Statements* elif_statements;
} Token_Data;

/* Tokens
 * Holds the count and head of the linked list holding args.
 * Populated by lexer.
 * data is populated by parser.
 */
typedef struct {
    size_t count;
    Token* head;
    Token_Data data;
} Tokens;
