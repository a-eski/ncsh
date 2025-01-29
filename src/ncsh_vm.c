// Copyright (c) ncsh by Alex Eski 2024

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <bits/types/siginfo_t.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_defines.h"
#include "ncsh_builtins.h"
#include "ncsh_defines.h"
#include "ncsh_parser.h"
#include "ncsh_terminal.h"
#include "ncsh_vm.h"

/* Types */
enum ncsh_Command_Type {
    CT_NONE = 0,
    CT_BUILTIN = 1,
    CT_EXTERNAL = 2
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

struct ncsh_Vm {
    struct ncsh_Output_Redirect_IO output_redirect_io;
    struct ncsh_Input_Redirect_IO input_redirect_io;
    struct ncsh_Pipe_IO pipes_io;
};

struct ncsh_Tokens {
    uint_fast32_t stdout_redirect_index;
    uint_fast32_t stdin_redirect_index;
    uint_fast32_t stderr_redirect_index;
    uint_fast32_t stdout_and_stderr_redirect_index;

    uint_fast32_t number_of_pipe_commands;

    char* stdout_file;
    char* stdin_file;
    char* stderr_file;
    char* stdout_and_stderr_file;

    bool output_append;
};

/* Builtins */
char* builtins[] = {NCSH_EXIT, NCSH_QUIT, NCSH_Q, NCSH_ECHO, NCSH_HELP, NCSH_CD, NCSH_PWD, NCSH_KILL, NCSH_SET};
size_t builtins_len[] = {sizeof(NCSH_EXIT), sizeof(NCSH_QUIT), sizeof(NCSH_Q),    sizeof(NCSH_ECHO), sizeof(NCSH_HELP),
                         sizeof(NCSH_CD),   sizeof(NCSH_PWD),  sizeof(NCSH_KILL), sizeof(NCSH_SET)};

int_fast32_t (*builtin_func[])(struct ncsh_Args*) = {&ncsh_builtins_exit, &ncsh_builtins_exit, &ncsh_builtins_exit,
                                                     &ncsh_builtins_echo, &ncsh_builtins_help, &ncsh_builtins_cd,
                                                     &ncsh_builtins_pwd,  &ncsh_builtins_kill, &ncsh_builtins_set};



/* Signal Handling */
static _Atomic pid_t ncsh_atomic_internal_child_pid = 0;

static inline void ncsh_vm_atomic_child_pid_set(pid_t pid)
{
    ncsh_atomic_internal_child_pid = pid;
}

static inline pid_t ncsh_vm_atomic_child_pid_get(void)
{
    return ncsh_atomic_internal_child_pid;
}

static void ncsh_vm_signal_handler(int signum, siginfo_t* info, void* context)
{
    (void)context;
    const pid_t target = ncsh_vm_atomic_child_pid_get();

    if (target != 0 && info->si_pid != target) {
        if (kill(target, signum) == 0) {
            if (write(STDOUT_FILENO, "\n", 1) == -1) { // write is async safe, do not use fflush, putchar, prinft
                perror(RED "ncsh: Error writing to standard output while processing a signal" RESET);
            }
        }
    }
}

static int ncsh_vm_signal_forward(const int signum)
{
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = ncsh_vm_signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(signum, &act, NULL)) {
        return errno;
    }

    return 0;
}

/* IO Redirection */
int ncsh_output_redirection_oflags_get(bool append)
{
    return append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
}

void ncsh_stdout_redirection_start(char* file, bool append, struct ncsh_Output_Redirect_IO* io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, ncsh_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stdout = -1;
    }

    io->original_stdout = dup(STDOUT_FILENO);
    dup2(file_descriptor, STDOUT_FILENO);

    close(file_descriptor);
}

void ncsh_stdout_redirection_stop(int original_stdout)
{
    dup2(original_stdout, STDOUT_FILENO);
}

void ncsh_stdin_redirection_start(char* file, struct ncsh_Input_Redirect_IO* io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, O_RDONLY);
    if (file_descriptor == -1) {
        io->fd = -1;
    }

    io->original_stdin = dup(STDIN_FILENO);
    dup2(file_descriptor, STDIN_FILENO);

    close(file_descriptor);
}

void ncsh_stdin_redirection_stop(int original_stdin)
{
    dup2(original_stdin, STDIN_FILENO);
}

void ncsh_stderr_redirection_start(char* file, bool append, struct ncsh_Output_Redirect_IO* io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, ncsh_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stderr = -1;
    }

    io->original_stderr = dup(STDERR_FILENO);
    dup2(file_descriptor, STDERR_FILENO);

    close(file_descriptor);
}

void ncsh_stderr_redirection_stop(int original_stderr)
{
    dup2(original_stderr, STDERR_FILENO);
}

void ncsh_stdout_and_stderr_redirection_start(char* file, bool append, struct ncsh_Output_Redirect_IO* io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, ncsh_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stdout = -1;
        io->fd_stderr = -1;
    }

    io->original_stdout = dup(STDOUT_FILENO);
    io->original_stderr = dup(STDERR_FILENO);
    dup2(file_descriptor, STDOUT_FILENO);
    dup2(file_descriptor, STDERR_FILENO);

    close(file_descriptor);
}

void ncsh_stdout_and_stderr_redirection_stop(struct ncsh_Output_Redirect_IO* io)
{
    assert(io);

    dup2(io->original_stdout, STDOUT_FILENO);
    dup2(io->original_stderr, STDERR_FILENO);
}

eskilib_nodiscard int_fast32_t ncsh_vm_redirection_start_if_needed(struct ncsh_Args* args, struct ncsh_Tokens* tokens,
                                                                   struct ncsh_Vm* vm)
{
    assert(args);
    assert(tokens);
    assert(vm);

    if (tokens->stdout_redirect_index && tokens->stdout_file) {
        free(args->values[tokens->stdout_redirect_index]);  // free the arg to remove it from list of args passed to the
                                                            // vm.
        args->values[tokens->stdout_redirect_index] = NULL; // set the arg to null to prevent double free
        ncsh_stdout_redirection_start(tokens->stdout_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for output redirection, do you have permission "
                   "to open the file?\n",
                   tokens->stdout_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stdin_redirect_index && tokens->stdin_file) {
        free(args->values[tokens->stdin_redirect_index]);
        args->values[tokens->stdin_redirect_index] = NULL;
        ncsh_stdin_redirection_start(tokens->stdin_file, &vm->input_redirect_io);
        if (vm->input_redirect_io.fd == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for input redirection, does the file exist?\n",
                   tokens->stdin_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stderr_redirect_index && tokens->stderr_file) {
        free(args->values[tokens->stderr_redirect_index]);
        args->values[tokens->stderr_redirect_index] = NULL;
        ncsh_stderr_redirection_start(tokens->stderr_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stderr == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for error redirection, does the file exist?\n",
                   tokens->stdin_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stdout_and_stderr_redirect_index && tokens->stdout_and_stderr_file) {
        free(args->values[tokens->stdout_and_stderr_redirect_index]);
        args->values[tokens->stdout_and_stderr_redirect_index] = NULL;
        ncsh_stdout_and_stderr_redirection_start(tokens->stdout_and_stderr_file, tokens->output_append,
                                                 &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for output & error redirection, does the file "
                   "exist?\n",
                   tokens->stdin_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void ncsh_vm_redirection_stop_if_needed(struct ncsh_Tokens* tokens, struct ncsh_Vm* vm)
{
    assert(tokens);
    assert(vm);

    if (tokens->stdout_redirect_index) {
        ncsh_stdout_redirection_stop(vm->output_redirect_io.original_stdout);
    }

    if (tokens->stdin_redirect_index) {
        ncsh_stdin_redirection_stop(vm->input_redirect_io.original_stdin);
    }

    if (tokens->stderr_redirect_index) {
        ncsh_stderr_redirection_stop(vm->output_redirect_io.original_stderr);
    }

    if (tokens->stdout_and_stderr_redirect_index) {
        ncsh_stdout_and_stderr_redirection_stop(&vm->output_redirect_io);
    }
}

/* Pipes */
eskilib_nodiscard int_fast32_t ncsh_pipe_start(uint_fast32_t command_position, struct ncsh_Pipe_IO* pipes)
{
    assert(pipes);

    if (command_position % 2 != 0) {
        if (pipe(pipes->fd_one) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }
    else {
        if (pipe(pipes->fd_two) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void ncsh_pipe_connect(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes)
{
    assert(pipes);

    if (command_position == 0) { // first command
        dup2(pipes->fd_two[1], STDOUT_FILENO);
    }
    else if (command_position == number_of_commands - 1) { // last command
        if (number_of_commands % 2 != 0) {
            dup2(pipes->fd_one[0], STDIN_FILENO);
        }
        else {
            dup2(pipes->fd_two[0], STDIN_FILENO);
        }
    }
    else { // middle command
        if (command_position % 2 != 0) {
            dup2(pipes->fd_two[0], STDIN_FILENO);
            dup2(pipes->fd_one[1], STDOUT_FILENO);
        }
        else {
            dup2(pipes->fd_one[0], STDIN_FILENO);
            dup2(pipes->fd_two[1], STDOUT_FILENO);
        }
    }
}

void ncsh_pipe_stop(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes)
{
    assert(pipes);

    if (command_position == 0) {
        close(pipes->fd_two[1]);
    }
    else if (command_position == number_of_commands - 1) {
        if (number_of_commands % 2 != 0) {
            close(pipes->fd_one[0]);
        }
        else {
            close(pipes->fd_two[0]);
        }
    }
    else {
        if (command_position % 2 != 0) {
            close(pipes->fd_two[0]);
            close(pipes->fd_one[1]);
        }
        else {
            close(pipes->fd_one[0]);
            close(pipes->fd_two[1]);
        }
    }
}

/* Tokenizing and Syntax Validation */
eskilib_nodiscard int_fast32_t ncsh_syntax_error(const char* message, size_t message_length)
{
    if (write(STDIN_FILENO, message, message_length) == -1) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }

    return NCSH_COMMAND_SYNTAX_ERROR;
}

#define INVALID_SYNTAX_PIPE_FIRST_ARG                                                                                  \
    "ncsh: Invalid syntax: found pipe operator ('|') as first argument. Correct usage of pipe operator is 'program1 "  \
    "| program2'.\n"
#define INVALID_SYNTAX_PIPE_LAST_ARG                                                                                   \
    "ncsh: Invalid syntax: found pipe operator ('|') as last argument. Correct usage of pipe operator is 'program1 | " \
    "program2'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found output redirection operator ('>') as first argument. Correct usage of output "        \
    "redirection operator is 'program > file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found no filename after output redirect operator ('>'). Correct usage of output "           \
    "redirection operator is 'program > file'.\n"

#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found output redirection append operator ('>>') as first argument. Correct usage of "       \
    "output redirection append operator is 'program >> file'.\n"
#define INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found no filename after output redirect append operator ('>>'). Correct usage of output "   \
    "redirection operator is 'program >> file'.\n"

#define INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG                                                                           \
    "ncsh: Invalid syntax: found input redirection operator ('<') as first argument. Correct usage of input "          \
    "redirection operator is 'program < file'.\n"
#define INVALID_SYNTAX_STDIN_REDIR_LAST_ARG                                                                            \
    "ncsh: Invalid syntax: found input redirection operator ('<') as last argument. Correct usage of input "           \
    "redirection operator is 'program < file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG                                                                          \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as first argument. Correct usage of error "         \
    "redirection is 'program 2> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_LAST_ARG                                                                           \
    "ncsh: Invalid syntax: found error redirection operator ('2>') as last argument. Correct usage of error "          \
    "redirection is 'program 2> file'.\n"

#define INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG                                                                   \
    "ncsh: Invalid syntax: found error redirection append operator ('2>>') as first argument. Correct usage of error " \
    "redirection is 'program 2>> file'.\n"
#define INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG                                                                    \
    "ncsh: Invalid syntax: found error redirection operator ('2>>') as last argument. Correct usage of error "         \
    "redirection is 'program 2>> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG                                                               \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as first argument. Correct usage of "      \
    "output & error redirection is 'program &> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG                                                                \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>') as last argument. Correct usage of "       \
    "output & error redirection is 'program &> file'.\n"

#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG                                                        \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as first argument. Correct usage of "     \
    "output & error redirection is 'program &>> file'.\n"
#define INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG                                                         \
    "ncsh: Invalid syntax: found output & error redirection operator ('&>>') as last argument. Correct usage of "      \
    "output & error redirection is 'program &>> file'.\n"

#define INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG                                                                        \
    "ncsh: Invalid syntax: found background job operator ('&') as first argument. Correct usage of background job "    \
    "operator is 'program &'.\n"

#define INVALID_SYNTAX(message) ncsh_syntax_error(message, sizeof(message) - 1)

eskilib_nodiscard int_fast32_t ncsh_vm_args_check(struct ncsh_Args* args)
{
    assert(args);

    switch (args->ops[0]) {
    case OP_PIPE: {
        return INVALID_SYNTAX(INVALID_SYNTAX_PIPE_FIRST_ARG);
    }
    case OP_STDOUT_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_FIRST_ARG);
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_FIRST_ARG);
    }
    case OP_STDIN_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_FIRST_ARG);
    }
    case OP_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_FIRST_ARG);
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_FIRST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_FIRST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_FIRST_ARG);
    }
    case OP_BACKGROUND_JOB: {
        return INVALID_SYNTAX(INVALID_SYNTAX_BACKGROUND_JOB_FIRST_ARG);
    }
    }

    switch (args->ops[args->count - 1]) {
    case OP_PIPE: {
        return INVALID_SYNTAX(INVALID_SYNTAX_PIPE_LAST_ARG);
    }
    case OP_STDOUT_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_LAST_ARG);
    }
    case OP_STDOUT_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_REDIR_APPEND_LAST_ARG);
    }
    case OP_STDIN_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDIN_REDIR_LAST_ARG);
    }
    case OP_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_LAST_ARG);
    }
    case OP_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDERR_REDIR_APPEND_LAST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_LAST_ARG);
    }
    case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
        return INVALID_SYNTAX(INVALID_SYNTAX_STDOUT_AND_STDERR_REDIR_APPEND_LAST_ARG);
    }
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

/*eskilib_nodiscard int_fast32_t ncsh_vm_tokens_check(struct ncsh_Tokens* tokens) {
    assert(tokens);

    if (tokens->stdout_redirect_index && tokens->stdin_redirect_index) {
        return ncsh_syntax_error("ncsh: Invalid syntax: found both input and output redirects operators ('<' and '>',
respectively).\n", 97);
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}*/

eskilib_nodiscard int_fast32_t ncsh_vm_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens)
{
    assert(args);
    assert(tokens);

    int_fast32_t syntax_check;
    if ((syntax_check = ncsh_vm_args_check(args)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return syntax_check;
    }

    for (uint_fast32_t i = 0; i < args->count; ++i) {
        switch (args->ops[i]) {
        case OP_STDOUT_REDIRECTION: {
            tokens->stdout_file = args->values[i + 1];
            tokens->stdout_redirect_index = i;
            break;
        }
        case OP_STDOUT_REDIRECTION_APPEND: {
            tokens->stdout_file = args->values[i + 1];
            tokens->stdout_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDIN_REDIRECTION: {
            tokens->stdin_file = args->values[i + 1];
            tokens->stdin_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION: {
            tokens->stderr_file = args->values[i + 1];
            tokens->stderr_redirect_index = i;
            break;
        }
        case OP_STDERR_REDIRECTION_APPEND: {
            tokens->stderr_file = args->values[i + 1];
            tokens->stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION: {
            tokens->stdout_and_stderr_file = args->values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            break;
        }
        case OP_STDOUT_AND_STDERR_REDIRECTION_APPEND: {
            tokens->stdout_and_stderr_file = args->values[i + 1];
            tokens->stdout_and_stderr_redirect_index = i;
            tokens->output_append = true;
            break;
        }
        case OP_PIPE: {
            ++tokens->number_of_pipe_commands;
            break;
        }
        }
    }
    ++tokens->number_of_pipe_commands;

    /*if ((syntax_check = ncsh_vm_tokens_check(tokens)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return syntax_check;
    }*/

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

/* Failure Handling */
eskilib_nodiscard int_fast32_t ncsh_fork_failure(uint_fast32_t command_position, uint_fast32_t number_of_commands,
                                                 struct ncsh_Pipe_IO* pipes)
{
    assert(pipes);

    if (command_position != number_of_commands - 1) {
        if (command_position % 2 != 0) {
            close(pipes->fd_one[1]);
        }
        else {
            close(pipes->fd_two[1]);
        }
    }

    perror(RED "ncsh: Error when forking process" RESET);
    fflush(stdout);
    return NCSH_COMMAND_EXIT_FAILURE;
}

/* VM */
#define EXECVP_FAILED -1

eskilib_nodiscard int_fast32_t ncsh_vm(struct ncsh_Args* args)
{
    assert(args);

    struct ncsh_Tokens tokens = {0};
    int_fast32_t result = ncsh_vm_tokenize(args, &tokens);
    if (result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    pid_t pid = 0;
    int status;
    int execvp_result = 0;
    int command_result = NCSH_COMMAND_NONE;
    struct ncsh_Vm vm = {0};
    char* buffer[MAX_INPUT] = {0};
    size_t buffer_len[MAX_INPUT] = {0};
    bool end = false;
    enum ncsh_Ops op_current = OP_NONE;
    enum ncsh_Command_Type command_type = CT_NONE;

    uint_fast32_t command_position = 0;
    uint_fast32_t args_position = 0;
    uint_fast32_t buffer_position = 0;

    if ((result = ncsh_vm_redirection_start_if_needed(args, &tokens, &vm)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    while (args->values[args_position] != NULL && end != true) {
        buffer_position = 0;

        while (args->ops[args_position] == OP_CONSTANT) {
            buffer[buffer_position] = args->values[args_position];
	    buffer_len[buffer_position] = args->lengths[args_position];
            ++args_position;

            if (args->values[args_position] == NULL) {
                end = true;
                ++buffer_position;
                break;
            }

            ++buffer_position;
        }

        if (!end) {
            op_current = args->ops[args_position];
        }
        ++args_position;

        buffer[buffer_position] = NULL;
        if (buffer[0] == NULL) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }

        if (op_current == OP_PIPE && !end) {
            if (!ncsh_pipe_start(command_position, &vm.pipes_io)) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
        }

        for (uint_fast32_t i = 0; i < sizeof(builtins) / sizeof(char*); ++i) {
            if (eskilib_string_compare(buffer[0], buffer_len[0], builtins[i], builtins_len[i])) {
                command_type = CT_BUILTIN;
                command_result = (*builtin_func[i])(args);

                if (op_current == OP_PIPE) {
                    ncsh_pipe_stop(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);
                }
                break;
            }
        }

        if (command_type != CT_BUILTIN) {
            /*if (ncsh_vm_signal_forward(SIGINT) ||
                ncsh_vm_signal_forward(SIGHUP) ||
                ncsh_vm_signal_forward(SIGTERM) ||
                ncsh_vm_signal_forward(SIGQUIT) ||
                ncsh_vm_signal_forward(SIGUSR1) ||
                ncsh_vm_signal_forward(SIGUSR2)) {
                perror("ncsh: Error setting up signal handlers");
                return NCSH_COMMAND_EXIT_FAILURE;
            }*/
            if (ncsh_vm_signal_forward(SIGINT)) {
                perror("ncsh: Error setting up signal handlers");
                return NCSH_COMMAND_EXIT_FAILURE;
            }

            pid = fork();

            if (pid == -1) {
                return ncsh_fork_failure(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);
            }

            if (pid == 0) {
                if (op_current == OP_PIPE) {
                    ncsh_pipe_connect(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);
                }

                /*if (setpgid(pid, pid) == 0) {
                    perror(RED "ncsh: Error setting up process group ID for child process" RESET);
                }*/

                if ((execvp_result = execvp(buffer[0], buffer)) == EXECVP_FAILED) {
                    end = true;
                    perror(RED "ncsh: Could not find command or directory" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (op_current == OP_PIPE) {
                ncsh_pipe_stop(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);
            }

            if (execvp_result == EXECVP_FAILED) {
                break;
            }

            ncsh_vm_atomic_child_pid_set(pid);

            __pid_t waitpid_result;
            while (1) {
                status = 0;
                waitpid_result = waitpid(pid, &status, WUNTRACED);

                // check for errors
                if (waitpid_result == -1) {
                    /* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
                    if (errno == EINTR) {
                        continue;
                    }

                    perror(RED "ncsh: Error waiting for child process to exit" RESET);
                    status = EXIT_FAILURE;
                    break;
                }

                // check if child process has exited
                if (waitpid_result == pid) {
#ifdef NCSH_DEBUG
                    if (WIFEXITED(status)) {
                        if (WEXITSTATUS(status)) {
                            fprintf(stderr, "ncsh: Command child process failed with status %d\n", WEXITSTATUS(status));
                        }
                        else {
                            fprintf(stderr, "ncsh: Command child process exited successfully.\n");
                        }
                    }
                    else if (WIFSIGNALED(status)) {
                        fprintf(stderr, "ncsh: Command child process died from signal #%d\n", WTERMSIG(status));
                    }
                    else {
                        if (write(STDERR_FILENO, "ncsh: Command child process died, cause unknown.\n", 49) == -1) {
                            perror("ncsh: Error writing to stderr");
                        }
                    }
#endif /* ifdef NCSH_DEBUG */

                    break;
                }
            }
        }

        ++command_position;
    }

    ncsh_vm_redirection_stop_if_needed(&tokens, &vm);

    if (execvp_result == EXECVP_FAILED) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (status == EXIT_FAILURE) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (command_result != NCSH_COMMAND_NONE)
        return command_result;

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void ncsh_vm_alias(struct ncsh_Args* args)
{
    struct eskilib_String alias = ncsh_config_alias_check(args->values[0], args->lengths[0]);
    if (alias.length) {
        args->values[0] = realloc(args->values[0], alias.length);
        memcpy(args->values[0], alias.value, alias.length - 1);
        args->values[0][alias.length - 1] = '\0';
        args->lengths[0] = alias.length;
    }
}

eskilib_nodiscard int_fast32_t ncsh_vm_execute(struct ncsh_Shell* shell)
{
    assert(shell);
    assert(&shell->args);

    if (shell->args.count == 0) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    if (eskilib_string_compare(shell->args.values[0], shell->args.lengths[0], NCSH_Z, sizeof(NCSH_Z))) {
        return ncsh_builtins_z(&shell->z_db, &shell->args);
    }

    if (eskilib_string_compare(shell->args.values[0], shell->args.lengths[0], NCSH_HISTORY, sizeof(NCSH_HISTORY))) {
        return ncsh_builtins_history(&shell->history, &shell->args);
    }

    ncsh_vm_alias(&shell->args);

    ncsh_terminal_os_reset(); // reset terminal settings since a lot of terminal programs use canonical mode
    int_fast32_t result = ncsh_vm(&shell->args);
    ncsh_terminal_os_init(); // back to noncanonical mode

    return result;
}

eskilib_nodiscard int_fast32_t ncsh_vm_execute_noninteractive(struct ncsh_Args* args)
{
    assert(args);
    if (!args->count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    ncsh_vm_alias(args);

    return ncsh_vm(args);
}
