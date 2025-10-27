/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_types.h: Types used during preprocessing and VM execution */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "parse.h"
#include "../types.h"

/****** MACROS ******/

/* VM_MAX_INPUT Macro constant
 * Size of VM buffer & buffer lengths, max number of char* VM can store and process */
#define VM_MAX_INPUT 64

/****** TYPES ******/
enum Vm_State {
    VS_NORMAL = 0,

    VS_IN_CONDITIONS,       // Generic in conditions, if/elif/while/for

    VS_IN_IF_STATEMENTS,    // statements in if/elif/else blocks
    VS_IN_ELSE_STATEMENTS,
    VS_IN_ELIF_STATEMENTS,

    VS_IN_LOOP_STATEMENTS,  // statements in for/while blocks
    VS_IN_LOOP_INIT,        // init statement for for loops
    VS_IN_LOOP_EACH_INIT,   // for each init
    VS_IN_LOOP_INCREMENT,   // increment/decrement for C style for loops
};

/* Output_Redirect_IO
 * Stores file descriptors (fds) for redirected output and original fds
 * for stdout and/or stderr */
typedef struct {
    int fd_stdout;
    int fd_stderr;
    int original_stdout;
    int original_stderr;
} Output_Redirect_IO;

/* Output_Redirect_IO
 * Stores file descriptors (fds) for redirected input and original fd
 * for stdin */
typedef struct {
    int fd;
    int original_stdin;
} Input_Redirect_IO;

/* Pipe_IO
 * Stores file descriptors (fds) and state for piping io between processes */
typedef struct {
    int fd_one[2];
    int fd_two[2];
} Pipe_IO;

/* Vm_Data
 * Stores information related to state in the VM.
 * Used in conjunction with Args and then Tokens. */
typedef struct {
    enum Ops op_current;
    enum Vm_State state;
    int status;

    uint8_t command_position;
    bool end;
    Statements* stmts;
    Statement* cur_stmt;
    Commands* cmds;
    Commands* next_cmds;
    size_t pos;

    Shell* sh;
    Arena* s;

    Output_Redirect_IO output_redirect_io;
    Input_Redirect_IO input_redirect_io;
    Pipe_IO pipes_io;
} Vm_Data;
