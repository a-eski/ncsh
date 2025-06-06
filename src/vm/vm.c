/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.c: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POSIX_C_SOURCE */

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
#include "pipe.h"
#include "preprocessor.h"
#include "redirection.h"
#include "syntax_validator.h"
#include "vm.h"
#include "vm_buffer.h"
#include "vm_types.h"

#ifdef NCSH_VM_TEST
#include "vm_mocks.h"
#endif /* NCSH_VM_TEST */

/* vm_child_pid: Used in signal handling, signals.h & main.c */
extern sig_atomic_t vm_child_pid;

/* vm_output_fd: set in pipe.c or redirection.c, read from builtins */
int vm_output_fd;
/* vm_error_fd: set in redirection.c, read from builtins */
// int vm_error_fd;

/*void vm_redirection_start_if_needed(Token_Data* rst tokens, Vm_Data* rst vm)
{
    redirection_start_if_needed(tokens, vm);
}*/

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
    return EXIT_FAILURE;
}

/* VM */
#define VM_COMMAND_DIED_MESSAGE "ncsh: Command child process died, cause unknown.\n"
void vm_status_check(Vm_Data* rst vm)
{
    if (WIFEXITED(vm->status)) {
        if ((vm->status = WEXITSTATUS(vm->status))) {
            fprintf(stderr, "ncsh: Command child process failed with status %d\n", WEXITSTATUS(vm->status));
        }
#ifdef NCSH_DEBUG
        else {
            fprintf(stderr, "ncsh: Command child process exited successfully.\n");
        }
#endif /* NCSH_DEBUG */
        // return EXIT_SUCCESS;
    }
#ifdef NCSH_DEBUG
    else if (WIFSIGNALED(vm->status)) {
        fprintf(stderr, "ncsh: Command child process died from signal %d\n", WTERMSIG(vm->status));
    }
    else {
        if (write(STDERR_FILENO, VM_COMMAND_DIED_MESSAGE, sizeof(VM_COMMAND_DIED_MESSAGE) - 1) == -1) {
            perror("ncsh: Error writing to stderr");
        }
    }
#endif /* NCSH_DEBUG */
}

void vm_status_set(int pid, Vm_Data* rst vm)
{
    pid_t waitpid_result;
    while (1) {
        vm->status = 0;
        waitpid_result = waitpid(pid, &vm->status, WUNTRACED);

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
        if (waitpid_result == pid) {
            // vm_status_check();
            break;
        }
    }
}

int vm_result_aggregate(Vm_Data* rst vm)
{
    if (vm->exec_result == EXECVP_FAILED) {
        return EXIT_FAILURE;
    }
    return vm->status;
}

[[nodiscard]]
int vm_run(Args* rst args, Token_Data* rst tokens, Shell* rst shell, Arena* rst scratch)
{
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(scratch, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(scratch, VM_MAX_INPUT, size_t);

    if (redirection_start_if_needed(tokens, &vm) != EXIT_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    Arg* arg = args->head->next;
    while (!vm.args_end && arg && arg->val) {
        Arg* next = vm_buffer_set(arg, tokens, &vm);
        if (next)
            arg = next;

        if (!vm.buffer[0]) {
            return EXIT_FAILURE_CONTINUE;
        }

        if (vm.op_current == OP_PIPE && !vm.args_end) {
            if (pipe_start(vm.command_position, &vm.pipes_io) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        bool builtin_ran = builtins_check_and_run(&vm, shell, scratch);
        if (builtin_ran) {
            vm.command_type = CT_BUILTIN;
            if (vm.op_current == OP_PIPE)
                pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);
        }

        if (vm.command_type == CT_EXTERNAL) {
            int vm_pid = fork();
            if (vm_pid < 0)
                return vm_fork_failure(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);

            if (vm_pid == 0) { // runs in the child process
                if (vm.op_current == OP_PIPE)
                    pipe_connect(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);

                if ((vm.exec_result = execvp(vm.buffer[0], vm.buffer)) == EXECVP_FAILED) {
                    vm.args_end = true;
                    perror(RED "ncsh: Could not run command" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (vm.op_current == OP_PIPE)
                pipe_stop(vm.command_position, tokens->number_of_pipe_commands, &vm.pipes_io);

            if (vm.exec_result == EXECVP_FAILED)
                break;

            vm_child_pid = vm_pid;
            vm_status_set(vm_pid, &vm);
        }

        if (vm.state == VS_IN_CONDITIONS && vm.status != EXIT_SUCCESS && tokens->logic_type != LT_IF_ELSE) {
            debug("breaking out of VM loop, condition failed.");
            vm.status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
            break;
        }

        if (vm.op_current == OP_AND && vm.status != EXIT_SUCCESS) {
            debug("breaking out of VM loop, short circuiting on AND.");
            vm.status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
            break;
        }

        if (vm.op_current == OP_OR && vm.status == EXIT_SUCCESS) {
            debug("breaking out of VM loop, short circuiting on OR.");
            break;
        }

        // TODO: logic around failures?
        // vm.status = vm_result_aggregate();?
        // if e is set?
        // logic? if next op is and/or?

        vm.command_type = CT_EXTERNAL;
        ++vm.command_position;
    }

    redirection_stop_if_needed(&vm);
    return vm_result_aggregate(&vm);
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
        return EXIT_SUCCESS;
    }

    // check if any jobs finished running
    /*if (shell->processes.job_number > 0) {
        vm_background_jobs_check(&shell->processes);
    }*/

    int result;
    if ((result = syntax_validator_validate(args)) != EXIT_SUCCESS)
        return result;

    Token_Data tokens = {0};
    if ((result = preprocessor_preprocess(args, &tokens, shell, scratch_arena)) != EXIT_SUCCESS) {
        return result;
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
        return EXIT_SUCCESS;
    }

    int result;
    if ((result = syntax_validator_validate(args)) != EXIT_SUCCESS)
        return result;

    Token_Data tokens = {0};
    if ((result = preprocessor_preprocess(args, &tokens, shell, &shell->arena)) != EXIT_SUCCESS) {
        return result;
    }

    return vm_run(args, &tokens, shell, &shell->arena);
}
