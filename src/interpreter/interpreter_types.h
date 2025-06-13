/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter_types.h: interpreter types for ncsh */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "tokens.h"
#include "ops.h"

/* Token
 * Holds op code, length, and value of the parsed token.
 * Includes a pointer to next element in the linked list.
 */
// typedef struct Token_ {
//     uint8_t op; // enum Ops
//     size_t len;
//     char* val;
//     struct Token_* next;
// } Token;

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
// typedef struct {
//     size_t count;
//     Token* head;
//     Token_Data data;
// } Tokens;
