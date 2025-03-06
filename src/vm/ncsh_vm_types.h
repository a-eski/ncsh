// Copyright (c) ncsh by Alex Eski 2025

#ifndef NCSH_VM_DATA_
#define NCSH_VM_DATA_

#include <fcntl.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdint.h>

#include "../ncsh_parser.h"

/* Macros */
#define EXECVP_FAILED -1

/* Types */
enum ncsh_Command_Type {
    CT_EXTERNAL = 0,
    CT_BUILTIN = 1
};

struct ncsh_Output_Redirect_IO {
    int fd_stdout;
    int fd_stderr;
    int original_stdout;
    int original_stderr;
};

struct ncsh_Input_Redirect_IO {
    int fd;
    int original_stdin;
};

struct ncsh_Pipe_IO {
    int fd_one[2];
    int fd_two[2];
};

struct ncsh_Vm_Data {
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
    enum ncsh_Ops op_current;
    enum ncsh_Command_Type command_type;
    struct ncsh_Output_Redirect_IO output_redirect_io;
    struct ncsh_Input_Redirect_IO input_redirect_io;
    struct ncsh_Pipe_IO pipes_io;
};

#endif // !NCSH_VM_DATA_
