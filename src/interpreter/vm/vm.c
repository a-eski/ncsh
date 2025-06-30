/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.c: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POSIX_C_SOURCE */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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
        debug("execvp failed");
        return EXIT_FAILURE;
    }
    if (vm->status == EXIT_FAILURE) {
        return EXIT_FAILURE_CONTINUE;
    }
    return vm->status;
}

[[nodiscard]]
bool vm_condition_failed(Vm_Data* rst vm, Statements* rst stmts)
{
    if (vm->state != VS_IN_CONDITIONS) {
        debug("vm_condition_failed false, not in conditions");
        return false;
    }

    bool condition_failed = vm->status != EXIT_SUCCESS && stmts->type != ST_IF_ELSE && vm->op_current != OP_OR;
    debugf("condition failed: %d\n", condition_failed);
    return condition_failed;
}

/*[[nodiscard]]
bool vm_status_should_break(Vm_Data* rst vm, Commands* cmds, Statements* rst stmts)
{
    (void)cmds;
    if (vm_condition_failed(vm, stmts)) {
        debug("breaking out of VM loop, condition failed.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        return true;
    }
    TODO: other logic around failures?
    // like if e is set?
    // logic? if next op is and/or?
    else {
        return false;
    }
}*/

[[nodiscard]]
int vm_math_process(Vm_Data* rst vm)
{
    debug("evaluating math conditions");
    char* c1 = vm->buffer[0];
    enum Ops op = vm->op_current;
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
        fprintf(stderr, "ncsh: while trying to process 'if' logic, found unsupported operation '%d'.", vm->op_current);
        result = false;
        break;
    }
    }

    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}

[[nodiscard]]
Commands* vm_command_next(Statements* rst stmts, Commands* rst cmds, Vm_Data* rst vm)
{
    if (!cmds->next || !cmds->next->vals[0]) {
        ++stmts->pos;
        if (stmts->pos >= stmts->count || stmts->pos >= stmts->cap) {
            debugf("hit stmts->count %zu or stmts->cap %zu, marking vm end\n", stmts->count, stmts->cap);
            vm->end = true;
            return NULL;
        }
        else if (vm->state == VS_IN_IF_STATEMENTS && stmts->statements[stmts->pos].type != LT_IF) {
            debug("at end of if statements, marking vm end");
            vm->end = true;
            return NULL;
        }
        else {
            if (!stmts->statements[stmts->pos].commands) {
                debug("no commands after incrementing stmts->pos, marking vm end");
                vm->end = true;
                return NULL;
            }
            debugf("setting cmds to next statment command at pos %zu\n", stmts->pos);
            cmds = stmts->statements[stmts->pos].commands;
            cmds->pos = 0;
            return cmds;
        }
    }

    cmds = cmds->next;
    cmds->pos = 0;
    return cmds;
}

[[nodiscard]]
Commands* vm_command_set(Statements* rst stmts, Commands* rst cmds, Vm_Data* rst vm, enum Vm_State state)
{
    vm->buffer = cmds->vals;
    vm->buffer_lens = cmds->lens;
    vm->state = state;
    vm->op_current = cmds->prev_op;
    return vm_command_next(stmts, cmds, vm);
}

[[nodiscard]]
Commands* vm_next_if(Statements* rst stmts, Commands* rst cmds, Vm_Data* rst vm)
{
    if (stmts->statements[stmts->pos].type == LT_CONDITIONS) {
        if (vm->state != VS_IN_CONDITIONS) {
            return vm_command_set(stmts, cmds, vm, VS_IN_CONDITIONS);
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_AND) {
            if (stmts->type == ST_IF_ELSE) {
                do {
                    ++stmts->pos;
                } while (stmts->pos < stmts->count && stmts->statements[stmts->pos].type != LT_ELSE);

                if (stmts->statements[stmts->pos].type != LT_ELSE ||
                    !stmts->statements[stmts->pos].commands) {
                    vm->end = true;
                    return NULL;
                }

                cmds = stmts->statements[stmts->pos].commands;
            }
            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_OR) {
                if (cmds->ops[cmds->pos] == OP_TRUE || cmds->ops[cmds->pos] == OP_CONSTANT) {
                    return vm_command_set(stmts, cmds, vm, VS_IN_CONDITIONS);
                }
                else if (stmts->type == ST_IF_ELSE) {
                    do {
                        ++stmts->pos;
                    } while (stmts->pos < stmts->count && stmts->statements[stmts->pos].type != LT_ELSE);

                    if (stmts->statements[stmts->pos].type != LT_ELSE ||
                        !stmts->statements[stmts->pos].commands) {
                        vm->end = true;
                        return NULL;
                    }

                    cmds = stmts->statements[stmts->pos].commands;
                }
                else {
                    vm->end = true;
                    return NULL;
                }
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (stmts->type == ST_IF_ELSE) {
                do {
                    ++stmts->pos;
                } while (stmts->pos < stmts->count && stmts->statements[stmts->pos].type != LT_ELSE);

                if (stmts->statements[stmts->pos].type != LT_ELSE ||
                    !stmts->statements[stmts->pos].commands) {
                    vm->end = true;
                    return NULL;
                }

                cmds = stmts->statements[stmts->pos].commands;
            }
            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status == EXIT_SUCCESS && cmds->prev_op == OP_OR) {
            do {
                ++stmts->pos;
            } while (stmts->pos < stmts->count && stmts->statements[stmts->pos].type != LT_IF);

            cmds = stmts->statements[stmts->pos].commands;
        }

        else {
            return vm_command_set(stmts, cmds, vm, VS_IN_CONDITIONS);
        }
    }

    if (stmts->statements[stmts->pos].type == LT_IF) {
        debug("in LT_IF");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS && stmts->type == ST_IF_ELSE) {
            if (stmts->statements[stmts->pos].type != LT_ELSE) {
                do {
                    ++stmts->pos;
                } while (stmts->pos < stmts->count && stmts->statements[stmts->pos].type != LT_ELSE);

                if (stmts->statements[stmts->pos].type != LT_ELSE ||
                    !stmts->statements[stmts->pos].commands) {
                    vm->end = true;
                    return NULL;
                }
            }

            cmds = stmts->statements[stmts->pos].commands;
            return vm_command_set(stmts, cmds, vm, VS_IN_ELSE_STATEMENTS);
        }
        if (vm->status != EXIT_SUCCESS && (vm->state == VS_IN_IF_STATEMENTS || vm->state == VS_IN_CONDITIONS) && cmds->prev_op != OP_AND) {

            vm->end = true;
            return NULL;
        }

        return vm_command_set(stmts, cmds, vm, VS_IN_IF_STATEMENTS);
    }

    if (stmts->statements[stmts->pos].type == LT_ELSE) {
        debug("in LT_ELSE");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_IF_STATEMENTS) {
            vm->end = true;
            return NULL;
        }

        if (vm->status == EXIT_SUCCESS && vm->state == VS_IN_ELSE_STATEMENTS) {
            return vm_command_set(stmts, cmds, vm, VS_IN_ELSE_STATEMENTS);
        }
        else if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS) {
            return vm_command_set(stmts, cmds, vm, VS_IN_ELSE_STATEMENTS);
        }
    }

    debug("no conditions met");
    debugf("vm->status %d\n", vm->status);
    vm->end = true;
    return NULL;
}

Commands* vm_next_normal(Statements* rst stmts, Commands* rst cmds, Vm_Data* rst vm)
{
    if (cmds && cmds->prev_op == OP_AND && vm->status != EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on AND.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        vm->end = true;
        return NULL;
    }
    else if (cmds && cmds->prev_op == OP_OR && vm->status == EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on OR.");
        vm->end = true;
        return NULL;
    }

    vm->buffer = cmds->vals;
    vm->buffer_lens = cmds->lens;
    cmds = vm_command_next(stmts, cmds, vm);

    if (!vm->end) {
        if ((cmds->prev_op == OP_PIPE || cmds->current_op == OP_PIPE)) {
            vm->op_current = OP_PIPE;
        }
        else {
            vm->op_current = cmds->prev_op;
        }
    }
    /*else if (!vm->end && (cmds->prev_op == OP_EQUALS || cmds->prev_op == OP_LESS_THAN || cmds->prev_op == OP_GREATER_THAN)) {
        vm->op_current = cmds->prev_op;
    }*/

    return cmds;
}

Commands* vm_next(Statements* rst stmts, Commands* cmds, Vm_Data* rst vm)
{
    assert(stmts);
    assert(vm);

    if (!cmds || stmts->pos >= stmts->count)
    {
        vm->end = true;
        return NULL;
    }

    switch (stmts->type) {
    case ST_IF:
    case ST_IF_ELSE:
    case ST_IF_ELIF_ELSE: {
        debug("vm_next_if");
        return vm_next_if(stmts, cmds, vm);
    }
    default: {
        debug("vm_next_normal");
        return vm_next_normal(stmts, cmds, vm);
    }
    }
}

[[nodiscard]]
int vm_run(Statements* rst stmts, Shell* rst shell, Arena* rst scratch)
{
    Vm_Data vm = {0};

    if (redirection_start_if_needed(stmts, &vm) != EXIT_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    stmts->pos = 0;
    Commands* cmds = stmts->statements[0].commands;
    cmds->pos = 0;

    while (!vm.end && cmds) {
        cmds = vm_next(stmts, cmds, &vm);

        if (!vm.buffer || !vm.buffer[0]) {
            return EXIT_FAILURE_CONTINUE;
        }

        if (vm.op_current == OP_PIPE && !vm.end) {
            if (pipe_start(vm.command_position, &vm.pipes_io) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
        }

        bool builtin_ran = builtins_check_and_run(&vm, shell, scratch);
        if (builtin_ran) {
            debugf("builtin ran %s\n", vm.buffer[0]);
            if (vm.op_current == OP_PIPE) {
                pipe_stop(vm.command_position, stmts->pipes_count, &vm.pipes_io);
            }
        }
        else if (vm.state == VS_IN_CONDITIONS &&
                 (vm.op_current == OP_EQUALS || vm.op_current == OP_GREATER_THAN || vm.op_current == OP_LESS_THAN)) {
            vm.status = vm_math_process(&vm);
        }
        else {
            int vm_pid = fork();
            if (vm_pid < 0)
                return vm_fork_failure(vm.command_position, stmts->pipes_count, &vm.pipes_io);

            if (vm_pid == 0) { // runs in the child process
                if (vm.op_current == OP_PIPE)
                    pipe_connect(vm.command_position, stmts->pipes_count, &vm.pipes_io);

                if ((vm.exec_result = execvp(vm.buffer[0], vm.buffer)) == EXECVP_FAILED) {
                    vm.end = true;
                    perror(RED "ncsh: Could not run command" RESET);
                    fflush(stdout);
                    kill(getpid(), SIGTERM);
                }
            }

            if (vm.op_current == OP_PIPE)
                pipe_stop(vm.command_position, stmts->pipes_count, &vm.pipes_io);

            if (vm.exec_result == EXECVP_FAILED)
                break;

            vm_child_pid = vm_pid;
            vm_status_set(vm_pid, &vm);
        }

        /*if (vm_status_should_break(&vm, cmds, stmts)) {
            debug("breaking bc of vm status");
            break;
        }*/

        ++vm.command_position;
    }

    redirection_stop_if_needed(&vm);
    return vm_status_aggregate(&vm);
}

/* vm_execute
 * Executes the VM in interactive mode.
 */
[[nodiscard]]
int vm_execute(Statements* rst stmts, Shell* rst shell, Arena* rst scratch)
{
    assert(shell);
    assert(stmts);

    if (!stmts || !stmts->count) {
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

    return vm_run(stmts, shell, scratch);
}

/* vm_execute_noninteractive
 * Executes the VM in noninteractive mode.
 * Please note that shell->arena is used for both perm & scratch arenas in noninteractive mode.
 */
[[nodiscard]]
int vm_execute_noninteractive(Statements* rst stmts, Shell* rst shell)
{
    assert(stmts);
    if (!stmts || !stmts->count) {
        return EXIT_SUCCESS;
    }

    return vm_run(stmts, shell, &shell->arena);
}
