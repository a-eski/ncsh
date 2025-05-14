/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include <fcntl.h>
#include <linux/limits.h>

#include "../parser.h"

/* Macros */
/* Result of system call execvp when it failes */
#define EXECVP_FAILED -1

/* Types */
/* enum Command_Type
 * Represents the command type, a shell builtin or external command that
 * needs to be called using fork/execvp
 */
enum Command_Type {
    CT_EXTERNAL = 0,
    CT_BUILTIN = 1
};

/* Output_Redirect_IO
 * Stores file descriptors (fds) for redirected output and original fds
 * for stdout and/or stderr
 */
typedef struct {
    int fd_stdout;
    int fd_stderr;
    int original_stdout;
    int original_stderr;
} Output_Redirect_IO;

/* Output_Redirect_IO
 * Stores file descriptors (fds) for redirected input and original fd
 * for stdin
 */
typedef struct {
    int fd;
    int original_stdin;
} Input_Redirect_IO;

/* Pipe_IO
 * Stores file descriptors (fds) and state for piping io between processes
 */
typedef struct {
    int current_output;
    int fd_one[2];
    int fd_two[2];
} Pipe_IO;

/* Vm_Data
 * Stores information related to state in the VM.
 * Used in conjunction with Args and then Tokens.
 */
typedef struct {
    char* buffer[MAX_INPUT];
    size_t buffer_lens[MAX_INPUT];
    uint8_t command_position;
    bool args_end;
    enum Ops op_current;
    Output_Redirect_IO output_redirect_io;
    Input_Redirect_IO input_redirect_io;
    Pipe_IO pipes_io;
} Vm_Data;
