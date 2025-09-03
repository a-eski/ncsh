/* Copyright ncsh (C) by Alex Eski 2025 */
/* stmts.h: statements and commands for ncsh */

#pragma once

#include <stdint.h>
#include <unistd.h>

#include "../eskilib/str.h"
#include "../types.h"
#include "lexemes.h"
#include "ops.h"

enum Redirect_Type : uint8_t {
    RT_NONE = 0,
    RT_OUT = 3,             // OP_STDOUT_REDIRECTION = 3,                       // >
    RT_OUT_APPEND = 4,      // OP_STDOUT_REDIRECTION_APPEND = 4,                // >>
    RT_IN = 5,              // OP_STDIN_REDIRECTION = 5,                        // <
    RT_IN_APPEND = 6,       // OP_STDIN_REDIRECTION_APPEND = 6,                 // <<
    RT_ERR = 7,             // OP_STDERR_REDIRECTION = 7,                       // 2>
    RT_ERR_APPEND = 8,      // OP_STDERR_REDIRECTION_APPEND = 8,                // 2>>
    RT_OUT_ERR = 9,         // OP_STDOUT_AND_STDERR_REDIRECTION = 9,            // &>
    RT_OUT_ERR_APPEND = 10, // OP_STDOUT_AND_STDERR_REDIRECTION_APPEND = 10,    // &>>
};

typedef struct Commands Commands;
struct Commands {
    size_t pos;
    size_t count;
    size_t cap;
    enum Ops prev_op;

    enum Ops* ops;
    Str* strs;

    Commands* next;
};

enum Logic_Type {
    LT_NORMAL = 0,
    LT_IF_CONDITIONS = 1,
    LT_IF,
    LT_ELSE,
    LT_ELIF_CONDITIONS,
    LT_ELIF
};

typedef struct Statement Statement;
struct Statement {
    enum Logic_Type type;
    Commands* commands;
    Statement* right;
    Statement* left;
    Statement* prev;
};

enum Statements_Type {
    ST_NORMAL = 0,
    ST_IF,
    ST_IF_ELSE,
    ST_IF_ELIF,
    ST_IF_ELIF_ELSE,
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
    Shell* restrict sh;
} Parser_Data;

Commands* cmds_alloc(Arena* restrict scratch);

void cmd_realloc(Commands* restrict cmds, Arena* restrict scratch);

void cmds_realloc(Parser_Data* restrict data, Arena* restrict scratch);

Commands* cmd_next(Commands* restrict cmds, Arena* restrict scratch);

Statement* stmt_alloc(Arena* restrict scratch);

Statements* stmts_alloc(Arena* restrict scratch);

void cmd_stmt_next(Parser_Data* data, enum Logic_Type type);
