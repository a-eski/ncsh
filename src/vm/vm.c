/* Copyright ncsh by Alex Eski 2024 */

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "../parser.h"
#include "../types.h"
#include "vm_builtins.h"
#include "vm_tokenizer.h"
#include "vm_types.h"

extern jmp_buf env;
extern sig_atomic_t vm_child_pid;

/* IO Redirection */
int vm_output_redirection_oflags_get(const bool append)
{
    return append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
}

void vm_stdout_redirection_start(const char* const restrict file, const bool append,
                                 struct Output_Redirect_IO* const restrict io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, vm_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stdout = -1;
    }

    io->original_stdout = dup(STDOUT_FILENO);
    dup2(file_descriptor, STDOUT_FILENO);

    close(file_descriptor);
}

void vm_stdout_redirection_stop(const int original_stdout)
{
    dup2(original_stdout, STDOUT_FILENO);
}

void vm_stdin_redirection_start(const char* const restrict file, struct Input_Redirect_IO* const restrict io)
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

void vm_stdin_redirection_stop(const int original_stdin)
{
    dup2(original_stdin, STDIN_FILENO);
}

void vm_stderr_redirection_start(const char* const restrict file, const bool append,
                                 struct Output_Redirect_IO* const restrict io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, vm_output_redirection_oflags_get(append), 0644);
    if (file_descriptor == -1) {
        io->fd_stderr = -1;
    }

    io->original_stderr = dup(STDERR_FILENO);
    dup2(file_descriptor, STDERR_FILENO);

    close(file_descriptor);
}

void vm_stderr_redirection_stop(const int original_stderr)
{
    dup2(original_stderr, STDERR_FILENO);
}

void vm_stdout_and_stderr_redirection_start(const char* const restrict file, const bool append,
                                            struct Output_Redirect_IO* const restrict io)
{
    assert(file);
    assert(io);

    int file_descriptor = open(file, vm_output_redirection_oflags_get(append), 0644);
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

void vm_stdout_and_stderr_redirection_stop(struct Output_Redirect_IO* const restrict io)
{
    assert(io);

    dup2(io->original_stdout, STDOUT_FILENO);
    dup2(io->original_stderr, STDERR_FILENO);
}

[[nodiscard]]
int_fast32_t vm_redirection_start_if_needed(struct Args* const restrict args,
                                            const struct Tokens* const restrict tokens,
                                            struct Vm_Data* const restrict vm)
{
    assert(args);
    assert(tokens);
    assert(vm);

    if (tokens->stdout_redirect_index && tokens->stdout_file) {
        args->values[tokens->stdout_redirect_index] = NULL;
        vm_stdout_redirection_start(tokens->stdout_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stdout == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for output redirection, do you have permission "
                   "to open the file?\n",
                   tokens->stdout_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stdin_redirect_index && tokens->stdin_file) {
        args->values[tokens->stdin_redirect_index] = NULL;
        vm_stdin_redirection_start(tokens->stdin_file, &vm->input_redirect_io);
        if (vm->input_redirect_io.fd == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for input redirection, does the file exist?\n",
                   tokens->stdin_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stderr_redirect_index && tokens->stderr_file) {
        args->values[tokens->stderr_redirect_index] = NULL;
        vm_stderr_redirection_start(tokens->stderr_file, tokens->output_append, &vm->output_redirect_io);
        if (vm->output_redirect_io.fd_stderr == -1) {
            printf("ncsh: Invalid file handle '%s': could not open file for error redirection, does the file exist?\n",
                   tokens->stdin_file);
            return NCSH_COMMAND_FAILED_CONTINUE;
        }
    }

    if (tokens->stdout_and_stderr_redirect_index && tokens->stdout_and_stderr_file) {
        args->values[tokens->stdout_and_stderr_redirect_index] = NULL;
        vm_stdout_and_stderr_redirection_start(tokens->stdout_and_stderr_file, tokens->output_append,
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

void vm_redirection_stop_if_needed(const struct Tokens* const restrict tokens, struct Vm_Data* const restrict vm)
{
    assert(tokens);
    assert(vm);

    if (tokens->stdout_redirect_index) {
        vm_stdout_redirection_stop(vm->output_redirect_io.original_stdout);
    }

    if (tokens->stdin_redirect_index) {
        vm_stdin_redirection_stop(vm->input_redirect_io.original_stdin);
    }

    if (tokens->stderr_redirect_index) {
        vm_stderr_redirection_stop(vm->output_redirect_io.original_stderr);
    }

    if (tokens->stdout_and_stderr_redirect_index) {
        vm_stdout_and_stderr_redirection_stop(&vm->output_redirect_io);
    }
}

/* Pipes */
[[nodiscard]]
int_fast32_t vm_pipe_start(const uint_fast32_t command_position, struct Pipe_IO* const restrict pipes)
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

void vm_pipe_connect(const uint_fast32_t command_position, const uint_fast32_t number_of_commands,
                     const struct Pipe_IO* const restrict pipes)
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
        }
        else {
            dup2(pipes->fd_one[0], STDIN_FILENO);
            dup2(pipes->fd_two[1], STDOUT_FILENO);
        }
    }
}

void vm_pipe_stop(const uint_fast32_t command_position, const uint_fast32_t number_of_commands,
                  const struct Pipe_IO* const restrict pipes)
{
    assert(pipes);

    if (!command_position) {
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

/* Failure Handling */
[[nodiscard]]
int_fast32_t vm_fork_failure(const uint_fast32_t command_position, const uint_fast32_t number_of_commands,
                             const struct Pipe_IO* const restrict pipes)
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
// Implementation not finished, still experimenting...
[[nodiscard]]
int_fast32_t vm_run_background_job(struct Args* const restrict args, struct Processes* const restrict processes,
                                   struct Tokens* const restrict tokens)
{
    (void)tokens;

    int execvp_result = NCSH_COMMAND_NONE;
    pid_t pid = fork();

    if (pid < 0) {
        perror(RED "ncsh: Error when forking process" RESET);
        fflush(stdout);
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    else if (pid == 0) {
        setsid();
        signal(SIGCHLD, SIG_DFL); // Restore default handler in child

        // Redirect standard input, output, and error
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        close(fd);

        if ((execvp_result = execvp(args->values[0], args->values)) == EXECVP_FAILED) {
            perror(RED "ncsh: Could not run command" RESET);
            fflush(stdout);
            kill(getpid(), SIGTERM);
            return NCSH_COMMAND_EXIT_FAILURE;
        }
    }
    else {
        signal(SIGCHLD, SIG_IGN); // Prevent zombie processes
        uint32_t job_number = !processes ? 0 : ++processes->job_number;
        printf("job [%u] pid [%d]\n", job_number, pid); // Job number and PID
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

/* VM */
#ifdef NCSH_DEBUG
#define VM_COMMAND_DIED_MESSAGE "ncsh: Command child process died, cause unknown.\n"
void vm_debug_status(struct Vm_Data* const restrict vm)
{
    if (WIFEXITED(vm->status)) {
        if (WEXITSTATUS(vm->status)) {
            fprintf(stderr, "ncsh: Command child process failed with status %d\n", WEXITSTATUS(vm->status));
        }
        else {
            fprintf(stderr, "ncsh: Command child process exited successfully.\n");
        }
    }
    else if (WIFSIGNALED(vm->status)) {
        fprintf(stderr, "ncsh: Command child process died from signal #%d\n", WTERMSIG(vm->status));
    }
    else {
        if (write(STDERR_FILENO, VM_COMMAND_DIED_MESSAGE, sizeof(VM_COMMAND_DIED_MESSAGE) - 1) == -1) {
            perror("ncsh: Error writing to stderr");
        }
    }
}
#endif /* ifdef NCSH_DEBUG */

void vm_buffer_set_command_next(struct Args* const restrict args, struct Vm_Data* const restrict vm)
{
    vm->buffer_position = 0;

    while (args->ops[vm->args_position] == OP_CONSTANT) {
        vm->buffer[vm->buffer_position] = args->values[vm->args_position];
        vm->buffer_len[vm->buffer_position] = args->lengths[vm->args_position];
        ++vm->args_position;

        if (!args->values[vm->args_position]) {
            vm->args_end = true;
            ++vm->buffer_position;
            break;
        }

        ++vm->buffer_position;
    }

    if (!vm->args_end) {
        vm->op_current = args->ops[vm->args_position];
    }
    ++vm->args_position;
}

void vm_waitpid(struct Vm_Data* vm)
{
    pid_t waitpid_result;
    while (1) {
        vm->status = 0;
        waitpid_result = waitpid(vm->pid, &vm->status, WUNTRACED);

        // check for errors
        if (waitpid_result == -1) {
            /* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
            if (errno == EINTR) {
                continue;
            }

            perror(RED "ncsh: Error waiting for child process to exit" RESET);
            vm->status = EXIT_FAILURE;
            break;
        }

        // check if child process has exited
        if (waitpid_result == vm->pid) {
#ifdef NCSH_DEBUG
            vm_debug_status(vm);
#endif /* ifdef NCSH_DEBUG */
            break;
        }
    }
}

[[nodiscard]]
int_fast32_t vm_run(struct Args* const restrict args, struct Tokens* const restrict tokens)
{
    int_fast32_t result;
    struct Vm_Data vm = {0};
    vm.command_result = NCSH_COMMAND_NONE;

    if ((result = vm_redirection_start_if_needed(args, tokens, &vm)) != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    while (args->values[vm.args_position] && vm.args_end != true) {
        vm_buffer_set_command_next(args, &vm);

        vm.buffer[vm.buffer_position] = NULL;
        if (!vm.buffer[0]) {
            return NCSH_COMMAND_FAILED_CONTINUE;
        }

        if (vm.op_current == OP_PIPE && !vm.args_end) {
            if (!vm_pipe_start(vm.command_position, &vm.pipes_io)) {
                return NCSH_COMMAND_EXIT_FAILURE;
            }
        }

        for (uint_fast32_t i = 0; i < builtins_count; ++i) {
            if (estrcmp_c(vm.buffer[0], vm.buffer_len[0], builtins[i].value, builtins[i].length)) {
                vm.command_type = CT_BUILTIN;
                vm.command_result = (*builtins[i].func)(args);

                if (vm.op_current == OP_PIPE) {
                    vm_pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
                }
                break;
            }
        }

        if (vm.command_type == CT_EXTERNAL) {
            vm.pid = fork();

            if (vm.pid < 0) {
                return vm_fork_failure(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
            }

            if (vm.pid == 0) { // runs in the child process
                if (vm.op_current == OP_PIPE) {
                    vm_pipe_connect(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
                }

                if ((vm.execvp_result = execvp(vm.buffer[0], vm.buffer)) == EXECVP_FAILED) {
                    vm.args_end = true;
                    perror(RED "ncsh: Could not run command" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (vm.op_current == OP_PIPE) {
                vm_pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
            }

            if (vm.execvp_result == EXECVP_FAILED) {
                break;
            }

            vm_child_pid = vm.pid;

            pid_t waitpid_result;
            while (1) {
                vm.status = 0;
                waitpid_result = waitpid(vm.pid, &vm.status, WUNTRACED);

                // check for errors
                if (waitpid_result == -1) {
                    /* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
                    if (errno == EINTR) {
                        continue;
                    }

                    perror(RED "ncsh: Error waiting for child process to exit" RESET);
                    vm.status = EXIT_FAILURE;
                    break;
                }

                // check if child process has exited
                if (waitpid_result == vm.pid) {
#ifdef NCSH_DEBUG
                    vm_debug_status(&vm);
#endif /* ifdef NCSH_DEBUG */
                    break;
                }
            }
        }

        vm.command_type = CT_EXTERNAL;
        ++vm.command_position;
    }

    vm_redirection_stop_if_needed(tokens, &vm);

    if (vm.execvp_result == EXECVP_FAILED) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (vm.status == EXIT_FAILURE) {
        return NCSH_COMMAND_EXIT_FAILURE;
    }
    if (vm.command_result != NCSH_COMMAND_NONE) {
        return vm.command_result;
    }

    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

/* vm_alias_replace
 * Replaces aliases with their aliased commands before calling main execute function on VM.
 */
void vm_alias_replace(struct Args* const restrict args, struct Arena* const scratch_arena)
{
    struct estr alias = config_alias_check(args->values[0], args->lengths[0]);
    if (alias.length) {
        args->values[0] = arena_realloc(scratch_arena, alias.length, char, args->values[0], args->lengths[0]);
        memcpy(args->values[0], alias.value, alias.length);
        args->lengths[0] = alias.length;
    }
}

/* vm_vars_replace
 * Replaces variables with their values before calling main execute function on VM.
 */
int_fast32_t vm_vars_replace(struct Args* const restrict args, struct Arena* scratch_arena)
{
    for (size_t i = 0; i < args->count; ++i) {
        if (args->ops[i] == OP_VARIABLE) {
            debugf("trying to get variable %s\n", args->values[i]);
            struct estr* val = var_get(args->values[i] + 1, &args->vars);
            if (!val) {
                printf("ncsh: variable with name '%s' did not have a value associated with it.\n", args->values[i]);
                return NCSH_COMMAND_FAILED_CONTINUE;
            }
            // replace the variable name with the value of the variable
            args->values[i] = arena_realloc(scratch_arena, val->length, char, args->values[i], args->lengths[i]);
            memcpy(args->values[i], val->value, val->length);
            args->lengths[i] = val->length;
            args->ops[i] = OP_CONSTANT; // replace OP_VARIABLE to OP_CONSTANT so VM sees it as a regular constant value
            debugf("replaced variable with value %s %zu\n", args->values[i], args->lengths[i]);
        }
    }
    return NCSH_COMMAND_SUCCESS_CONTINUE;
}

[[nodiscard]]
int_fast32_t vm_execute(struct Shell* const restrict shell, struct Arena* const scratch_arena)
{
    assert(shell);
    assert(&shell->args);

    if (!shell->args.count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    // check if any jobs finished running

    // check for builtins that run outside of the main VM execute function (z, history)
    /* TODO:: these don't work with aliases, variables, pipes and other operators as a result, need to incorporate into
     * the VM execute function. */
    if (estrcmp_c(shell->args.values[0], shell->args.lengths[0], Z, sizeof(Z))) {
        return builtins_z(&shell->z_db, &shell->args, &shell->arena, scratch_arena);
    }

    if (estrcmp_c(shell->args.values[0], shell->args.lengths[0], NCSH_HISTORY, sizeof(NCSH_HISTORY))) {
        return builtins_history(&shell->input.history, &shell->args, &shell->arena, scratch_arena);
    }

    vm_alias_replace(&shell->args, &shell->scratch_arena);
    int_fast32_t result = vm_vars_replace(&shell->args, &shell->scratch_arena);
    if (result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    struct Tokens tokens = {0};
    result = vm_tokenizer_tokenize(&shell->args, &tokens);
    if (result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    if (tokens.is_background_job == true) {
        return vm_run_background_job(&shell->args, &shell->processes, &tokens);
    }

    return vm_run(&shell->args, &tokens);
}

[[nodiscard]]
int_fast32_t vm_execute_noninteractive(struct Args* const restrict args, struct Arena* const arena)
{
    assert(args);
    if (!args->count) {
        return NCSH_COMMAND_SUCCESS_CONTINUE;
    }

    vm_alias_replace(args, arena);

    struct Tokens tokens = {0};
    int_fast32_t result = vm_tokenizer_tokenize(args, &tokens);
    if (result != NCSH_COMMAND_SUCCESS_CONTINUE) {
        return result;
    }

    return vm_run(args, &tokens);
}
