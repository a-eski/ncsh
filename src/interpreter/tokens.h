/* Copyright ncsh (C) by Alex Eski 2025 */
/* args.h: a linked list for storing tokens outputted by the lexer */

#pragma once

#include <stddef.h>
#include <stdint.h>

#include "../arena.h"
#include "interpreter_types.h"

typedef struct {
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
    // enum Logic_Type logic_type;
    // Statements* conditions;
    // Statements* if_statements;
    // Statements* else_statements;
    // Statements* elif_statements;
} Token_Data;

typedef struct Token_ {
    uint8_t op;
    size_t pos;
    size_t count;
    size_t* lens;
    char** vals;
    struct Token_* next;
} Token;

typedef struct {
    size_t count;
    Token* head;
    Token_Data* data;
} Tokens;

Tokens* tokens_alloc(Arena* rst arena);

Token* token_alloc(uint8_t op, size_t len, char* rst val, Arena* rst arena);

bool token_set_after(Token* rst current, Token* rst after);

bool token_set_last(Tokens* rst toks, Token* rst last);
