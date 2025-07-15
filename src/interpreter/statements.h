#pragma once

#include <stdint.h>
#include <unistd.h>

#include "../arena.h"
#include "../defines.h"
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

typedef struct Commands_ {
    size_t pos;
    size_t count;
    size_t cap;
    enum Ops current_op;
    enum Ops prev_op;

    enum Ops* ops;
    size_t* lens;
    char** vals;

    struct Commands_* next;
} Commands;

enum Logic_Type {
    LT_NORMAL = 0,
    LT_IF_CONDITIONS = 1,
    LT_IF,
    LT_ELSE,
    LT_ELIF_CONDTIONS,
    LT_ELIF
};

typedef struct {
    enum Logic_Type type;
    size_t count;
    Commands* commands;
} Statement;

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
    uint8_t pipes_count;

    size_t pos;
    size_t count;
    size_t cap;
    enum Statements_Type type;
    Statement* statements;
} Statements;

Commands* commands_alloc(Arena* rst scratch);

void command_realloc(Commands* rst cmds, Arena* rst scratch);
void commands_realloc(Statements* rst stmts, Arena* rst scratch);

Commands* command_next(Commands* rst cmds, Arena* rst scratch);
Commands* command_statement_next(Statements* rst stmts, Commands* cmds, enum Logic_Type type, Arena* rst scratch);

void statements_init(Statements* rst stmts, Arena* rst scratch);

void statement_next(Statements* rst stmts, enum Logic_Type type, Arena* rst scratch);
