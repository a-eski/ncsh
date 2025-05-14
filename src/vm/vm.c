/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.c: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../args.h"
#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "../types.h"
#include "builtins.h"
#include "vm.h"
#include "vm_tokenizer.h"
#include "vm_types.h"

#ifdef NCSH_VM_TEST
#include "vm_mocks.h"
#endif /* NCSH_VM_TEST */

extern sig_atomic_t vm_child_pid;

/* IO Redirection */
extern inline int vm_output_redirection_oflags_get(bool append);

void vm_stdout_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd_stdout = open(file, vm_output_redirection_oflags_get(append), 0644);
    if (io->fd_stdout == -1) {
        fprintf(stderr, "ncsh: Invalid file handle '%s': could not open file for output redirection.\n", file);
        perror("ncsh: file error");
        return;
    }

    io->original_stdout = dup(STDOUT_FILENO);
    dup2(io->fd_stdout, STDOUT_FILENO);

    close(io->fd_stdout);
}

void vm_stdout_redirection_stop(int original_stdout)
{
    assert(original_stdout >= 0);
    if (original_stdout < 0)
        return;
    dup2(original_stdout, STDOUT_FILENO);
}

void vm_stdin_redirection_start(char* rst file, Input_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd = open(file, O_RDONLY);
    if (io->fd == -1) {
        fprintf(stderr, "ncsh: Invalid file handle '%s': could not open file for input redirection.\n", file);
        perror("ncsh: file error");
        return;
    }

    io->original_stdin = dup(STDIN_FILENO);
    dup2(io->fd, STDIN_FILENO);

    close(io->fd);
}

void vm_stdin_redirection_stop(int original_stdin)
{
    dup2(original_stdin, STDIN_FILENO);
}

void vm_stderr_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    io->fd_stderr = open(file, vm_output_redirection_oflags_get(append), 0644);
    if (io->fd_stderr == -1) {
        fprintf(stderr, "ncsh: Invalid file handle '%s': could not open file for error redirection.\n", file);
        perror("ncsh: file error");
        return;
    }

    io->original_stderr = dup(STDERR_FILENO);
    dup2(io->fd_stderr, STDERR_FILENO);

    close(io->fd_stderr);
}

void vm_stderr_redirection_stop(int original_stderr)
{
    assert(original_stderr >= 1);
    if (original_stderr < 0)
        return;
    dup2(original_stderr, STDERR_FILENO);
}

void vm_stdout_and_stderr_redirection_start(char* rst file, bool append, Output_Redirect_IO* rst io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, vm_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stdout = -1;
        io->fd_stderr = -1;
        fprintf(stderr, "ncsh: Invalid file handle '%s': could not open file for ouput & error redirection.\n", file);
        perror("ncsh: file error");
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

void vm_stdout_and_stderr_redirection_stop(Output_Redirect_IO* rst io)
{
    assert(io);

    dup2(io->original_stdout, STDOUT_FILENO);
    dup2(io->original_stderr, STDERR_FILENO);
}

[[nodiscard]]
int vm_redirection_start_if_needed(Tokens* rst tokens, Vm_Data* rst vm)
{
    assert(tokens);
    assert(vm);

    if (tokens->stdout_redirect && tokens->stdout_file) {
        tokens->stdout_redirect->val = NULL;
        vm_stdout_redirection_start(tokens->stdout_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        debug("started stdout redirection");
    }

    if (tokens->stdin_redirect && tokens->stdin_file) {
        tokens->stdin_redirect->val = NULL;
        vm_stdin_redirection_start(tokens->stdin_file, &vm->input_redirect_io);
        if (vm->input_redirect_io.fd == -1) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        debug("started stdin redirection");
    }

    if (tokens->stderr_redirect && tokens->stderr_file) {
        tokens->stderr_redirect->val = NULL;
        vm_stderr_redirection_start(tokens->stderr_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stderr == -1) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        debug("started stderr redirection");
    }

    if (tokens->stdout_and_stderr_redirect && tokens->stdout_and_stderr_file) {
        tokens->stdout_and_stderr_redirect->val = NULL;
        vm_stdout_and_stderr_redirection_start(tokens->stdout_and_stderr_file, tokens->output_append,
                                               &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
        debug("started stdout and stderr redirection");
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void vm_redirection_stop_if_needed(Vm_Data* rst vm)
{
    assert(vm);

    if (vm->output_redirect_io.fd_stdout > 0) {
        vm_stdout_redirection_stop(vm->output_redirect_io.original_stdout);
        debug("stopped stdout redirection");
    }

    if (vm->input_redirect_io.fd > 0) {
        vm_stdin_redirection_stop(vm->input_redirect_io.original_stdin);
        debug("stopped stdin redirection");
    }

    if (vm->output_redirect_io.fd_stderr > 0) {
        vm_stderr_redirection_stop(vm->output_redirect_io.original_stderr);
        vm->output_redirect_io.fd_stderr = 0;
        debug("stopped stderr redirection");
    }

    if (vm->output_redirect_io.fd_stdout > 0 && vm->output_redirect_io.fd_stderr > 0) {
        vm_stdout_and_stderr_redirection_stop(&vm->output_redirect_io);
        debug("stopped stdout and stderr redirection");
    }
}

/* Pipes */
[[nodiscard]]
int vm_pipe_start(size_t command_position, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (command_position % 2 != 0) {
        if (pipe(pipes->fd_one) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        pipes->current_output = pipes->fd_one[1];
    }
    else {
        if (pipe(pipes->fd_two) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
        pipes->current_output = pipes->fd_two[1];
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void vm_pipe_connect(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (!command_position) { // first command
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
            pipes->current_output = pipes->fd_one[1];
        }
        else {
            dup2(pipes->fd_one[0], STDIN_FILENO);
            dup2(pipes->fd_two[1], STDOUT_FILENO);
            pipes->current_output = pipes->fd_two[1];
        }
    }
}

void vm_pipe_stop(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (!command_position) {
        close(pipes->fd_two[1]);
    }
    else if (command_position == number_of_commands - 1) {
        if (number_of_commands % 2 != 0) {
            close(pipes->fd_one[0]);
            pipes->current_output = STDOUT_FILENO;
        }
        else {
            close(pipes->fd_two[0]);
            pipes->current_output = STDOUT_FILENO;
        }
    }
    else {
        if (command_position % 2 != 0) {
            close(pipes->fd_two[0]);
            close(pipes->fd_one[1]);
            pipes->current_output = STDOUT_FILENO;
        }
        else {
            close(pipes->fd_one[0]);
            close(pipes->fd_two[1]);
            pipes->current_output = STDOUT_FILENO;
        }
    }
}

/* Failure Handling */
[[nodiscard]]
int vm_fork_failure(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes)
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

/* Background Jobs */
// Implementation not working, still experimenting...
/*[[nodiscard]]
int vm_background_job_run(Args* rst args, Processes* rst processes,
                          Tokens* rst tokens)
{
    assert(processes);
    (void)tokens;

    int execvp_result = NCSH_COMMAND_NONE;
    pid_t pid = fork();

    if (pid < 0) {
        perror(RED "ncsh: Error when forking process" RESET);
        fflush(stdout);
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    else if (pid == 0) { // runs in the child process
        setsid();
        signal(SIGCHLD, SIG_DFL); // Restore default handler in child

        // Redirect standard input, output, and error
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        char cmds[PARSER_TOKENS_LIMIT][NCSH_MAX_INPUT];
        Arg* arg = args->head->next;
        for (size_t i = 0; i < args->count && arg; ++i) {
            memcpy(cmds[i], arg->val, arg->len);
            arg = arg->next;
        }

        if ((execvp_result = execvp(cmds[0], (char**)cmds)) == EXECVP_FAILED) {
            perror(RED "ncsh: Could not run command" RESET);
            fflush(stdout);
            kill(getpid(), SIGTERM);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }
    else {
        signal(SIGCHLD, SIG_IGN); // Prevent zombie processes
        size_t job_number = ++processes->job_number;
        printf("job [%zu] pid [%d]\n", job_number, pid);
        processes->pids[job_number - 1] = pid;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}*/

/*void vm_background_jobs_check(Processes* rst processes)
{
    assert(processes);
    (void)processes;

    for (size_t i = 0; i < processes->job_number; ++i) {
        pid_t waitpid_result;
        while (1) {
            int status = 0;
            waitpid_result = waitpid(processes->pids[0], &status, WUNTRACED);

            // check for errors
            if (waitpid_result == -1) {
                // ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags
                if (errno == EINTR) {
                    continue;
                }

                perror(RED "ncsh: Error waiting for job child process to exit" RESET);
                break;
            }
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status)) {
                    fprintf(stderr, "ncsh: Command child process returned with status %d\n", WEXITSTATUS(status));
                }
#ifdef NCSH_DEBUG
                else {
                    fprintf(stderr, "ncsh: Command child process exited successfully.\n");
                }
#endif // NCSH_DEBUG
            }
        }
    }
}*/

/* VM */
int vm_status;
int vm_command_result;
enum Command_Type vm_command_type;
int vm_execvp_result;
int vm_result;
int vm_pid;

#define VM_COMMAND_DIED_MESSAGE "ncsh: Command child process died, cause unknown.\n"
void vm_status_check()
{
    if (WIFEXITED(vm_status)) {
        if (WEXITSTATUS(vm_status)) {
            fprintf(stderr, "ncsh: Command child process failed with status %d\n", WEXITSTATUS(vm_status));
        }
#ifdef NCSH_DEBUG
        else {
            fprintf(stderr, "ncsh: Command child process exited successfully.\n");
        }
#endif /* NCSH_DEBUG */
        // return EXIT_SUCCESS;
    }
#ifdef NCSH_DEBUG
    else if (WIFSIGNALED(vm_status)) {
        fprintf(stderr, "ncsh: Command child process died from signal %d\n", WTERMSIG(vm_status));
    }
    else {
        if (write(STDERR_FILENO, VM_COMMAND_DIED_MESSAGE, sizeof(VM_COMMAND_DIED_MESSAGE) - 1) == -1) {
            perror("ncsh: Error writing to stderr");
        }
    }
#endif /* NCSH_DEBUG */
}

Arg* vm_buffer_set_command_next(Arg* rst arg, Vm_Data* rst vm)
{
    if (!arg) {
        vm->buffer[0] = NULL;
        vm->args_end = true;
    }

    if (arg->op == OP_ASSIGNMENT) {
        arg = arg->next;
        if (!arg) {
            vm->buffer[0] = NULL;
            vm->args_end = true;
        }
    }

    if (arg && (arg->op == OP_AND || arg->op == OP_OR)) {
        arg = arg->next;
        if (!arg) {
            vm->buffer[0] = NULL;
            vm->args_end = true;
        }
    }

    if (!arg) {
        vm->buffer[0] = NULL;
        vm->args_end = true;
    }

    size_t vm_buf_pos = 0;
    while (arg && arg->val && arg->op == OP_CONSTANT) {
        assert(arg->val[arg->len - 1] == '\0');
        vm->buffer[vm_buf_pos] = arg->val;
        vm->buffer_lens[vm_buf_pos] = arg->len;
        debugf("set vm->buffer[%zu] to %s, %zu\n", vm_buf_pos, arg->val, arg->len);

        if (!arg->next || !arg->next->val) {
            debug("at end of args");
            vm->args_end = true;
            ++vm_buf_pos;
            break;
        }

        ++vm_buf_pos;
        arg = arg->next;
    }

    if (!vm->args_end) {
        vm->op_current = arg->op;
        debugf("set op current to %d\n", vm->op_current);
        arg = arg->next;
    }

    vm->buffer[vm_buf_pos] = NULL;

    return arg;
}

int vm_result_aggregate()
{
    if (vm_execvp_result == EXECVP_FAILED) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (vm_status == EXIT_FAILURE) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (vm_command_result != NCSH_COMMAND_NONE) {
        return vm_command_result;
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

/* vm_builtins_check_and_run
 * Checks current command against builtins, and if matches runs the builtin.
 */
[[nodiscard]]
bool vm_builtins_check_and_run(Vm_Data* rst vm, Shell* rst shell, Arena* rst scratch_arena)
{
    if (shell) {
        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], Z, sizeof(Z))) {
            vm_command_result = builtins_z(&shell->z_db, vm->buffer, vm->buffer_lens, &shell->arena, scratch_arena);
            return true;
        }

        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], NCSH_HISTORY, sizeof(NCSH_HISTORY))) {
            vm_command_result =
                builtins_history(&shell->input.history, vm->buffer, vm->buffer_lens, &shell->arena, scratch_arena);
            return true;
        }
    }

    for (size_t i = 0; i < builtins_count; ++i) {
        if (estrcmp(vm->buffer[0], vm->buffer_lens[0], builtins[i].value, builtins[i].length)) {
            vm_command_result = (*builtins[i].func)(vm->buffer);
            return true;
        }
    }

    return false;
}

[[nodiscard]]
int vm_run(Args* rst args, Tokens* rst tokens, Shell* rst shell, Arena* rst scratch_arena)
{
    Vm_Data vm = {.pipes_io = {.current_output = STDOUT_FILENO}};

    if ((vm_result = vm_redirection_start_if_needed(tokens, &vm)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return vm_result;
    }

    Arg* arg = args->head->next;
    while (!vm.args_end && arg && arg->val) {
        vm_command_result = NCSH_COMMAND_NONE;
        arg = vm_buffer_set_command_next(arg, &vm);

        if (!vm.buffer[0]) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }

        if (vm.op_current == OP_PIPE && !vm.args_end) {
            if (!vm_pipe_start(vm.command_position, &vm.pipes_io)) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
        }

        bool builtin_ran = vm_builtins_check_and_run(&vm, shell, scratch_arena);
        if (builtin_ran) {
            vm_command_type = CT_BUILTIN;
            if (vm.op_current == OP_PIPE) {
                vm_pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
            }
        }

        if (vm_command_type == CT_EXTERNAL) {
            vm_pid = fork();

            if (vm_pid < 0) {
                return vm_fork_failure(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
            }

            if (vm_pid == 0) { // runs in the child process
                if (vm.op_current == OP_PIPE) {
                    vm_pipe_connect(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
                }

                if ((vm_execvp_result = execvp(vm.buffer[0], vm.buffer)) == EXECVP_FAILED) {
                    vm.args_end = true;
                    perror(RED "ncsh: Could not run command" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (vm.op_current == OP_PIPE) {
                vm_pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
            }

            if (vm_execvp_result == EXECVP_FAILED) {
                break;
            }

            vm_child_pid = vm_pid;

            pid_t waitpid_result;
            while (1) {
                vm_status = 0;
                waitpid_result = waitpid(vm_pid, &vm_status, WUNTRACED);

                // check for errors
                if (waitpid_result == -1) {
                    /* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
                    if (errno == EINTR) {
                        continue;
                    }

                    perror(RED "ncsh: Error waiting for child process to exit" RESET);
                    vm_status = EXIT_FAILURE;
                    break;
                }

                // check if child process has exited
                if (waitpid_result == vm_pid) {
                    vm_status_check();
                    break;
                }
            }
        }

        vm_command_type = CT_EXTERNAL;
        ++vm.command_position;

        // TODO: logic around failures
        // vm_result = vm_result_aggregate();
        // if e is set
        // if next op is and/or
    }

    vm_redirection_stop_if_needed(&vm);

    vm_result = vm_result_aggregate();
    return vm_result;
}

/* vm_execute
 * Executes the VM in interactive mode.
 */
[[nodiscard]]
int vm_execute(Args* rst args, Shell* rst shell, Arena* rst scratch_arena)
{
    assert(shell);
    assert(args);

    if (!args || !args->head || !args->head->next || !args->count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    // check if any jobs finished running
    /*if (shell->processes.job_number > 0) {
        vm_background_jobs_check(&shell->processes);
    }*/

    Tokens tokens = {0};
    vm_result = vm_tokenizer_tokenize(args, &tokens, shell, scratch_arena);
    if (vm_result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return vm_result;
    }

    // TODO: implement background jobs
    /*if (tokens.is_background_job == true) {
        return vm_background_job_run(args, &shell->processes, &tokens);
    }*/

    return vm_run(args, &tokens, shell, scratch_arena);
}

/* vm_execute_noninteractive
 * Executes the VM in noninteractive mode.
 * Please note that shell->arena is used for both perm & scratch arenas in noninteractive mode.
 */
[[nodiscard]]
int vm_execute_noninteractive(Args* rst args, Shell* rst shell)
{
    assert(args);
    if (!args || !args->head || !args->head->next || !args->count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    Tokens tokens = {0};
    vm_result = vm_tokenizer_tokenize(args, &tokens, shell, &shell->arena);
    if (vm_result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return vm_result;
    }

    return vm_run(args, &tokens, shell, &shell->arena);
}
