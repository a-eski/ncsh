/* Copyright ncsh by Alex Eski 2025 */

#pragma once

#include <fcntl.h>
#include <linux/limits.h>
#include <stdint.h>

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

/* struct Output_Redirect_IO
 * Stores file descriptors (fds) for redirected output and original fds
 * for stdout and/or stderr
 */
struct Output_Redirect_IO {
    int fd_stdout;
    int fd_stderr;
    int original_stdout;
    int original_stderr;
};

/* struct Output_Redirect_IO
 * Stores file descriptors (fds) for redirected input and original fd
 * for stdin
 */
struct Input_Redirect_IO {
    int fd;
    int original_stdin;
};

/* struct Pipe_IO
 * Stores file descriptors (fds) for piping io between processes
 */
struct Pipe_IO {
    int fd_one[2];
    int fd_two[2];
};

/* struct Vm_Data
 * Stores information related to state in the VM.
 * Used in conjunction with struct Args and then struct Tokens.
 */
struct Vm_Data {
    pid_t pid;
    int status;
    int execvp_result;
    int command_result;
    char* buffer[MAX_INPUT];
    size_t buffer_len[MAX_INPUT];
    uint_fast32_t buffer_position;
    uint_fast32_t command_position;
    uint_fast32_t args_position;
    bool args_end;
    enum Ops op_current;
    enum Command_Type command_type;
    struct Output_Redirect_IO output_redirect_io;
    struct Input_Redirect_IO input_redirect_io;
    struct Pipe_IO pipes_io;
};
