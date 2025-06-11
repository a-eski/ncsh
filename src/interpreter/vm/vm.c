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

#include "../../defines.h"
#include "../../eskilib/ecolors.h"
#include "../../types.h"
#include "builtins.h"
#include "pipe.h"
#include "redirection.h"
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

[[nodiscard]]
int vm_status_aggregate(Vm_Data* rst vm)
{
    if (vm->exec_result == EXECVP_FAILED) {
        return EXIT_FAILURE;
    }
    return vm->status;
}

bool vm_condition_failed(Vm_Data* rst vm, Token_Data* rst data)
{
    if (vm->state != VS_IN_CONDITIONS)
        return false;

    return vm->status != EXIT_SUCCESS && data->logic_type != LT_IF_ELSE && vm->op_current != OP_OR;
}

bool vm_status_should_break(Vm_Data* rst vm, Token_Data* rst data)
{
    if (vm_condition_failed(vm, data)) {
        debug("breaking out of VM loop, condition failed.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        return true;
    }
    else if (vm->op_current == OP_AND && vm->status != EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on AND.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        return true;
    }
    else if (vm->op_current == OP_OR && vm->status == EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on OR.");
        return true;
    }
    // TODO: other logic around failures?
    // like if e is set?
    // logic? if next op is and/or?
    else {
        return false;
    }
}

[[nodiscard]]
int vm_math_process(Vm_Data* rst vm)
{
    char* c1 = vm->buffer[0];
    enum Ops op = vm->ops[1];
    char* c2 = vm->buffer[2];

    bool result;
    switch (op) {
    case OP_EQUALS: {
        result = atoi(c1) == atoi(c2);
        break;
    }
    case OP_LESS_THAN: {
        result = atoi(c1) < atoi(c2);
        break;
    }
    case OP_GREATER_THAN: {
        result = atoi(c1) > atoi(c2);
        break;
    }
    default: {
        puts("ncsh: while trying to process 'if' logic, found unsupported operation.");
        result = false;
        break;
    }
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

[[nodiscard]]
int vm_run(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    Vm_Data vm = {0};
    vm.buffer = arena_malloc(scratch, VM_MAX_INPUT, char*);
    vm.buffer_lens = arena_malloc(scratch, VM_MAX_INPUT, size_t);

    if (redirection_start_if_needed(&toks->data, &vm) != EXIT_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    Token* tok = toks->head->next;
    while (!vm.tokens_end && tok && tok->val) {
        Token* next = vm_buffer_set(tok, &toks->data, &vm);
        if (next)
            tok = next;

        if (!vm.buffer[0]) {
            return EXIT_FAILURE_CONTINUE;
        }

        if (vm.op_current == OP_PIPE && !vm.tokens_end) {
            if (pipe_start(vm.command_position, &vm.pipes_io) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        bool builtin_ran = builtins_check_and_run(&vm, shell, scratch);
        if (builtin_ran) {
            if (vm.op_current == OP_PIPE)
                pipe_stop(vm.command_position, toks->data.number_of_pipe_commands, &vm.pipes_io);
        }
        else if (VS_IN_CONDITIONS && vm.ops &&
                 (vm.ops[1] == OP_EQUALS || vm.ops[1] == OP_GREATER_THAN || vm.ops[1] == OP_LESS_THAN)) {
            vm.status = vm_math_process(&vm);
        }
        else {
            int vm_pid = fork();
            if (vm_pid < 0)
                return vm_fork_failure(vm.command_position, toks->data.number_of_pipe_commands, &vm.pipes_io);

            if (vm_pid == 0) { // runs in the child process
                if (vm.op_current == OP_PIPE)
                    pipe_connect(vm.command_position, toks->data.number_of_pipe_commands, &vm.pipes_io);

                if ((vm.exec_result = execvp(vm.buffer[0], vm.buffer)) == EXECVP_FAILED) {
                    vm.tokens_end = true;
                    perror(RED "ncsh: Could not run command" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (vm.op_current == OP_PIPE)
                pipe_stop(vm.command_position, toks->data.number_of_pipe_commands, &vm.pipes_io);

            if (vm.exec_result == EXECVP_FAILED)
                break;

            vm_child_pid = vm_pid;
            vm_status_set(vm_pid, &vm);
        }

        if (vm_status_should_break(&vm, &toks->data))
            break;

        ++vm.command_position;
    }

    redirection_stop_if_needed(&vm);
    return vm_status_aggregate(&vm);
}

/* vm_execute
 * Executes the VM in interactive mode.
 */
[[nodiscard]]
int vm_execute(Tokens* rst toks, Shell* rst shell, Arena* rst scratch)
{
    assert(shell);
    assert(toks);

    if (!toks || !toks->head || !toks->head->next || !toks->count) {
        return EXIT_SUCCESS;
    }

    // check if any jobs finished running
    /*if (shell->processes.job_number > 0) {
        vm_background_jobs_check(&shell->processes);
    }*/

    // TODO: implement background jobs
    /*if (tokens.is_background_job == true) {
        return vm_background_job_run(toks, &shell->processes, data);
    }*/

    return vm_run(toks, shell, scratch);
}

/* vm_execute_noninteractive
 * Executes the VM in noninteractive mode.
 * Please note that shell->arena is used for both perm & scratch arenas in noninteractive mode.
 */
[[nodiscard]]
int vm_execute_noninteractive(Tokens* rst toks, Shell* rst shell)
{
    assert(toks);
    if (!toks || !toks->head || !toks->head->next || !toks->count) {
        return EXIT_SUCCESS;
    }

    return vm_run(toks, shell, &shell->arena);
}
