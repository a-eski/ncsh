/* Copyright ncsh (C) by Alex Eski 2025 */
/* parser.h: Preprocessing of lexer output to produce data in a format ready for the VM to process. */

#pragma once

#include "lex.h"

/* enum Ops
 * Represents the bytecodes which get sent to the VM.
 */
enum Ops : uint8_t {
    // Default value, indicative of an issue parsing when found during execution
    OP_NONE = 0,
    // Constant values
    OP_CONST,
    OP_NUM,
    // Boolean
    OP_TRUE,                                  // true
    OP_FALSE,                                 // false
    // Shell operators
    OP_PIPE,                                  // |
    OP_AND,                                   // &&
    OP_OR,                                    // ||
    // Math
    OP_ADD,                                   // +
    OP_SUB,                                   // -
    OP_MUL,                                   // *
    OP_DIV,                                   // /
    OP_MOD,                                   // %
    OP_EXP,                                   // **
    OP_MATH_EXPR_START,                       // $( or $((
    OP_MATH_EXPR_END,                         // ) or ))
    // Equality comparisons
    OP_EQUALS,                                // -eq
    OP_LESS_THAN,                             // -lt
    OP_LESS_THAN_OR_EQUALS,                   // -le
    OP_GREATER_THAN,                          // -gt
    OP_GREATER_THAN_OR_EQUALS,                // -ge
    // Expansions
    OP_VARIABLE,                              // (starting with $, $VAR)
    OP_ASSIGNMENT,                            // (var=val)
    OP_HOME_EXPANSION,                        // ~
    OP_GLOB_EXPANSION,                        // * or ?

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    // these could be condensed/removed into fewer ops.
    // they are not needed by vm, only by parser to characterize tokens.
    // If Control flow Structures
    OP_CONDITION_START,                       // [, [[
    OP_CONDITION_END,                         // ], ]]
    OP_IF,                                    // if
    OP_ELSE,                                  // else
    OP_ELIF,                                  // elif
    OP_THEN,                                  // then
    OP_FI,                                    // fi
    // Loop Control Flow Structures
    OP_WHILE,                                 // while
    OP_FOR,                                   // for
    OP_DO,                                    // do
    OP_DONE                                   // done
};

enum Redirect_Type : uint8_t {
    RT_NONE = 0,
    RT_OUT = 3,             // >
    RT_OUT_APPEND = 4,      // >>
    RT_IN = 5,              // <
    RT_IN_APPEND = 6,       // <<
    RT_ERR = 7,             // 2>
    RT_ERR_APPEND = 8,      // 2>>
    RT_OUT_ERR = 9,         // &>
    RT_OUT_ERR_APPEND = 10, // &>>
};

typedef struct Commands Commands;
struct Commands {
    size_t pos;
    size_t count;
    size_t cap;

    enum Ops* ops;
    Str* strs;
    Str* keys;

    Commands* next;
    enum Ops op;
    enum Ops prev_op;
};

enum Logic_Type {
    LT_NORMAL = 0,
    LT_IF_CONDITIONS = 1,
    LT_IF,
    LT_ELSE,
    LT_ELIF_CONDITIONS,
    LT_ELIF,
    LT_WHILE,
    LT_WHILE_CONDITIONS
};

typedef struct Statement Statement;
struct Statement {
    Commands* commands;
    Statement* right;
    Statement* left;
    Statement* prev;
    enum Logic_Type type;
};

enum Statements_Type {
    ST_NORMAL = 0,
    ST_IF,
    ST_IF_ELSE,
    ST_IF_ELIF,
    ST_IF_ELIF_ELSE,
    ST_WHILE,
    ST_FOR
};

typedef struct {
    enum Redirect_Type redirect_type;
    char* redirect_filename;
    bool is_bg_job;
    uint8_t pipes_count; // counts the number of commands, not pipes.

    enum Statements_Type type;
    Statement* head;
} Statements;

typedef struct {
    Lexemes* restrict lexemes;
    Statements* restrict stmts;
    Statement* restrict cur_stmt;
    Statement* restrict prev_stmt;
    Commands* restrict cur_cmds;
    Arena* restrict s;
    Str_Builder* restrict sb;
} Parser_Data;

enum {
    PE_NOTHING = 1,
    PE_INVALID_STMT,
    PE_MISSING_TOK
};

typedef struct {
    int parser_errno;
    union {
        char* msg;
        Statements* stmts;
    } output;
} Parser_Output;

void cmd_realloc_exact(Commands* restrict cmds, Arena* restrict scratch, size_t new_cap);

Parser_Output parse(Lexemes* lexemes, Arena* restrict scratch);
