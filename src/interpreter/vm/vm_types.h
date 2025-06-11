/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_types.h: Types used during preprocessing and VM execution */

#pragma once

#include <stdint.h>

#include "../tokens.h"
#include "../../eskilib/str.h"

/****** MACROS ******/

/* VM_MAX_INPUT Macro constant
 * Size of VM buffer & buffer lengths, max number of char* VM can store and process */
#define VM_MAX_INPUT 64

/* EXECVP_FAILED Macro constant
 * Result of system call execvp when it fails */
#define EXECVP_FAILED -1

/****** TYPES ******/
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
    enum Logic_Type logic_type;
    Statements* conditions;
    Statements* if_statements;
    Statements* else_statements;
    // Statements* elif_statements;
} Token_Data;

/*** VM TYPES ***/
enum Vm_State {
    VS_NORMAL = 0,
    VS_IN_CONDITIONS,
    VS_IN_IF_STATEMENTS,
    VS_IN_ELSE_STATEMENTS
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
    char** buffer;
    size_t* buffer_lens;
    enum Ops* ops;
    uint8_t command_position;
    bool tokens_end;

    enum Ops op_current;
    enum Vm_State state;

    size_t conditions_pos;
    size_t if_statment_pos;
    size_t else_statment_pos;
    // size_t elif_statment_pos;

    int status;
    int exec_result;

    Output_Redirect_IO output_redirect_io;
    Input_Redirect_IO input_redirect_io;
    Pipe_IO pipes_io;
} Vm_Data;
