/* Copyright ncsh (C) by Alex Eski 2025 */
/* redirection.c: IO Redirection */

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include "../../debug.h"
#include "../../defines.h"
#include "../statements.h"
#include "../../ttyterm/ttyterm.h"
#include "vm_types.h"

[[nodiscard]]
static inline int output_redirection_oflags_get(bool append)
{
    return append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
}

void stdout_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd_stdout = open(file, output_redirection_oflags_get(append), 0644);
    if (io->fd_stdout == -1) {
        term_fprintln(stderr, "ncsh: Invalid file handle '%s': could not open file for output redirection.", file);
        term_perror("ncsh: file error");
        return;
    }

    io->original_stdout = dup(STDOUT_FILENO);
    dup2(io->fd_stdout, STDOUT_FILENO);

    close(io->fd_stdout);
}

void stdout_redirection_stop(int original_stdout)
{
    assert(original_stdout >= 0);
    if (original_stdout < 0)
        return;
    dup2(original_stdout, STDOUT_FILENO);
}

void stdin_redirection_start(char* rst file, Input_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd = open(file, O_RDONLY);
    if (io->fd == -1) {
        term_fprintln(stderr, "ncsh: Invalid file handle '%s': could not open file for input redirection.", file);
        term_perror("ncsh: file error");
        return;
    }

    io->original_stdin = dup(STDIN_FILENO);
    dup2(io->fd, STDIN_FILENO);

    close(io->fd);
}

void stdin_redirection_stop(int original_stdin)
{
    dup2(original_stdin, STDIN_FILENO);
}

void stderr_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd_stderr = open(file, output_redirection_oflags_get(append), 0644);
    if (io->fd_stderr == -1) {
        term_fprintln(stderr, "ncsh: Invalid file handle '%s': could not open file for error redirection.", file);
        term_perror("ncsh: file error");
        return;
    }

    io->original_stderr = dup(STDERR_FILENO);
    dup2(io->fd_stderr, STDERR_FILENO);

    close(io->fd_stderr);
}

void stderr_redirection_stop(int original_stderr)
{
    assert(original_stderr >= 1);
    if (original_stderr < 0)
        return;
    dup2(original_stderr, STDERR_FILENO);
}

void stdout_and_stderr_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stdout = -1;
        io->fd_stderr = -1;
        term_fprintln(stderr, "ncsh: Invalid file handle '%s': could not open file for ouput & error redirection.", file);
        term_perror("ncsh: file error");
        return;
    }
    io->fd_stdout = file_descriptor;
    io->fd_stderr = file_descriptor;

    io->original_stdout = dup(STDOUT_FILENO);
    io->original_stderr = dup(STDERR_FILENO);
    dup2(file_descriptor, STDOUT_FILENO);
    dup2(file_descriptor, STDERR_FILENO);

    close(file_descriptor);
}

void stdout_and_stderr_redirection_stop(Output_Redirect_IO* rst io)
{
    assert(io);

    dup2(io->original_stdout, STDOUT_FILENO);
    dup2(io->original_stderr, STDERR_FILENO);
}

[[nodiscard]]
int redirection_start_if_needed(Statements* rst stmts, Vm_Data* rst vm)
{
    assert(stmts);
    assert(vm);

    switch (stmts->redirect_type) {
    case RT_OUT:
    case RT_OUT_APPEND: {
        stdout_redirection_start(stmts->redirect_filename, stmts->redirect_type == RT_OUT_APPEND,
                                 &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            return EXIT_FAILURE_CONTINUE;
        }
        debug("started stdout redirection");
        break;
    }
    case RT_IN:
    case RT_IN_APPEND: {
        // TODO: support stdin append redirection
        stdin_redirection_start(stmts->redirect_filename, &vm->input_redirect_io);
        if (vm->input_redirect_io.fd == -1) {
            return EXIT_FAILURE_CONTINUE;
        }
        debug("started stdin redirection");
        break;
    }
    case RT_ERR:
    case RT_ERR_APPEND: {
        stderr_redirection_start(stmts->redirect_filename, stmts->redirect_type == RT_ERR_APPEND,
                                 &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stderr == -1) {
            return EXIT_FAILURE_CONTINUE;
        }
        debug("started stderr redirection");
        break;
    }
    case RT_OUT_ERR:
    case RT_OUT_ERR_APPEND: {
        stdout_and_stderr_redirection_start(stmts->redirect_filename,
                                            stmts->redirect_type == RT_OUT_ERR_APPEND, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            return EXIT_FAILURE_CONTINUE;
        }
        debug("started stdout and stderr redirection");
        break;
    }
    case RT_NONE: {
        return EXIT_SUCCESS;
    }
    }

    return EXIT_SUCCESS;
}

void redirection_stop_if_needed(Vm_Data* rst vm)
{
    assert(vm);

    if (vm->output_redirect_io.fd_stdout > 0) {
        stdout_redirection_stop(vm->output_redirect_io.original_stdout);
        debug("stopped stdout redirection");
    }

    if (vm->input_redirect_io.fd > 0) {
        stdin_redirection_stop(vm->input_redirect_io.original_stdin);
        debug("stopped stdin redirection");
    }

    if (vm->output_redirect_io.fd_stderr > 0) {
        stderr_redirection_stop(vm->output_redirect_io.original_stderr);
        vm->output_redirect_io.fd_stderr = 0;
        debug("stopped stderr redirection");
    }

    if (vm->output_redirect_io.fd_stdout > 0 && vm->output_redirect_io.fd_stderr > 0) {
        stdout_and_stderr_redirection_stop(&vm->output_redirect_io);
        debug("stopped stdout and stderr redirection");
    }
}
