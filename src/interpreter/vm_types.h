/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_types.h: Types used during preprocessing and VM execution */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "ops.h"
#include "../types.h"
#include "../eskilib/str.h"
#include "stmts.h"

/****** MACROS ******/

/* VM_MAX_INPUT Macro constant
 * Size of VM buffer & buffer lengths, max number of char* VM can store and process */
#define VM_MAX_INPUT 64

/****** TYPES ******/
enum Vm_State {
    VS_NORMAL = 0,
    VS_IN_CONDITIONS,
    VS_IN_IF_STATEMENTS,
    VS_IN_ELSE_STATEMENTS,
    VS_IN_ELIF_STATEMENTS,
    VS_IN_LOOP_STATEMENTS
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
    size_t strs_n;
    Str* strs;
    enum Ops* ops;

    enum Ops op_current;
    enum Vm_State state;
    int status;

    size_t loop_c_n;
    Str* loop_c_s;
    enum Ops* loop_c_ops;
    enum Ops loop_c_op;

    uint8_t command_position;
    bool end;
    Statements* stmts;
    Statement* cur_stmt;
    Commands* cur_cmds;

    Shell* sh;
    Arena* s;

    Output_Redirect_IO output_redirect_io;
    Input_Redirect_IO input_redirect_io;
    Pipe_IO pipes_io;
} Vm_Data;
