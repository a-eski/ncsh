/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.c: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../defines.h"
#include "../signals.h"
#include "../types.h"
#include "../debug.h"
#include "../ttyio/ttyio.h"
#include "stmts.h"
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
int vm_fork_failure(size_t command_position, size_t number_of_commands, Pipe_IO* restrict pipes)
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

    tty_perror("ncsh: Error when forking process");
    return EXIT_FAILURE;
}

/* VM */
#define VM_COMMAND_DIED_MESSAGE "ncsh: Command child process died, cause unknown."
void vm_status_check(Vm_Data* restrict vm)
{
    if (WIFEXITED(vm->status)) {
        if ((vm->status = WEXITSTATUS(vm->status))) {
            tty_fprintln(stderr, "ncsh: Command child process failed with status %d", WEXITSTATUS(vm->status));
        }
#ifdef NCSH_DEBUG
        else {
            tty_fprintln(stderr, "ncsh: Command child process exited successfully.");
        }
#endif /* NCSH_DEBUG */
        // return EXIT_SUCCESS;
    }
#ifdef NCSH_DEBUG
    else if (WIFSIGNALED(vm->status)) {
        tty_fprintln(stderr, "ncsh: Command child process died from signal %d", WTERMSIG(vm->status));
    }
    else {
        tty_writeln(VM_COMMAND_DIED_MESSAGE, sizeof(VM_COMMAND_DIED_MESSAGE) - 1);
    }
#endif /* NCSH_DEBUG */
}

void vm_waitpid(int pid, Vm_Data* restrict vm)
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

            tty_perror("ncsh: Error waiting for child process to exit");
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
int vm_status_aggregate(Vm_Data* restrict vm)
{
    if (vm->status == EXIT_FAILURE) {
        return EXIT_FAILURE_CONTINUE;
    }
    return vm->status;
}

/* vm_math_process
 * Sets vm->status with result of operation
 */
void vm_math_process(Vm_Data* restrict vm)
{
    debug("evaluating math conditions");

#ifdef NCSH_DEBUG
    char** buf = vm->buffer;
    while (*buf && printf("%s\n", *buf) && ++buf);
#endif

    if (!vm->strs || !vm->strs[0].value || !vm->strs[2].value) {
        vm->status = EXIT_FAILURE_CONTINUE;
        return;
    }

    char* c1 = vm->strs[0].value;
    enum Ops op = vm->op_current;
    char* c2 = vm->strs[2].value;
    assert(c1); assert(c2);

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
        tty_fprintln(stderr, "ncsh: while trying to process 'if' logic, found unsupported operation '%d'.", vm->op_current);
        result = false;
        break;
    }
    }

    vm->status = result ? EXIT_SUCCESS : EXIT_FAILURE;
}

[[nodiscard]]
int vm_command_next(Vm_Data* restrict vm)
{
    if (!vm->cur_cmds->next || ! vm->cur_cmds->next->strs[0].value) {
        if (!vm->cur_stmt->right) {
            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }
        else if (vm->state == VS_IN_IF_STATEMENTS && vm->cur_stmt->right->type != LT_IF) {
            debug("at end of if statements, marking vm end");
            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }
        else {
            if (!vm->cur_stmt->right->commands) {
                debug("no commands after incrementing stmts->pos, marking vm end");
                vm->end = true;
                return EXIT_FAILURE_CONTINUE;
            }
            vm->cur_stmt = vm->cur_stmt->right;
            vm->cur_cmds = vm->cur_stmt->commands;
            vm->cur_cmds->pos = 0;
            return EXIT_SUCCESS;
        }
    }

    vm->cur_cmds = vm->cur_cmds->next;
    vm->cur_cmds->pos = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_command_set(Vm_Data* restrict vm, enum Vm_State state)
{
    vm->strs = vm->cur_cmds->strs;
    vm->strs_n = vm->cur_cmds->count;
    vm->state = state;
    vm->op_current = vm->cur_cmds->prev_op;
    return vm_command_next(vm);
}

[[nodiscard]]
int vm_next_if_statement(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->right;
    if (vm->cur_stmt->type != LT_IF || !vm->cur_stmt->commands) {
        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }
    vm->cur_cmds = vm->cur_stmt->commands;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_next_else_statement(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->left;
    if (vm->cur_stmt->type != LT_ELSE || !vm->cur_stmt->commands) {
        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }
    vm->cur_cmds = vm->cur_stmt->commands;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_next_elif_condition(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->left;
    if (vm->cur_stmt->type != LT_ELIF_CONDITIONS || !vm->cur_stmt->commands) {
        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }
    vm->cur_cmds = vm->cur_stmt->commands;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_next_elif_statement(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->right;
    if (vm->cur_stmt->type != LT_ELIF || !vm->cur_stmt->commands) {
        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }
    vm->cur_cmds = vm->cur_stmt->commands;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_next_if(Vm_Data* restrict vm)
{
    if (vm->cur_stmt->type == LT_IF_CONDITIONS) {
        if (vm->state != VS_IN_CONDITIONS) {
            return vm_command_set(vm, VS_IN_CONDITIONS);
        }

        else if (vm->status != EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_AND) {
            if (vm->stmts->type == ST_IF_ELSE) {
                if (vm_next_else_statement(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }
                return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
            }
            else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                if (vm_next_elif_condition(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }

                return vm_command_set(vm, VS_IN_CONDITIONS);
            }
            else {
                vm->end = true;
                return EXIT_FAILURE_CONTINUE;
            }
        }

        else if (vm->status != EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_OR) {
                if (vm->cur_cmds->ops[vm->cur_cmds->pos] == OP_TRUE || vm->cur_cmds->ops[vm->cur_cmds->pos] == OP_CONST) {
                    return vm_command_set(vm, VS_IN_CONDITIONS);
                }

                else if (vm->cur_stmt->type == LT_IF_CONDITIONS) {
                    return vm_command_set(vm, VS_IN_CONDITIONS);
                }

                else if (vm->stmts->type == ST_IF_ELSE) {
                    if (vm_next_else_statement(vm)) {
                        return EXIT_FAILURE_CONTINUE;
                    }

                    return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
                }

                else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                    debug("going to next elif condition");
                    if (vm_next_elif_condition(vm)) {
                        return EXIT_FAILURE_CONTINUE;
                    }

                    return vm_command_set(vm, VS_IN_CONDITIONS);
                }

                else {
                    vm->end = true;
                    return EXIT_FAILURE_CONTINUE;
                }
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->stmts->type == ST_IF_ELSE) {
                if (vm_next_else_statement(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }
                return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
            }

            else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                debug("going to next elif condition");
                if (vm_next_elif_condition(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }

                return vm_command_set(vm, VS_IN_CONDITIONS);
            }

            else {
                vm->end = true;
                return EXIT_FAILURE_CONTINUE;
            }
        }

        else if (vm->status == EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_OR) {
            if (vm_next_if_statement(vm)) {
                return EXIT_FAILURE_CONTINUE;
            }

            return vm_command_set(vm, VS_IN_IF_STATEMENTS);
        }

        else {
            return vm_command_set(vm, VS_IN_CONDITIONS);
        }
    }

    if (vm->cur_stmt->type == LT_IF) {
        debug("in LT_IF");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS && vm->stmts->type == ST_IF_ELSE) {
            if (vm->cur_stmt->type != LT_ELSE) {
                if (vm_next_else_statement(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }
            }

            // cmds = stmts->statements[stmts->pos].commands;
            return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
        }

        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS && (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE)) {
            debug("in LT_IF ST_IF_ELIF or ST_IF_ELIF_ELSE");
            if (vm->cur_stmt->type != LT_ELIF_CONDITIONS) {
                if (vm_next_elif_condition(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }

                return vm_command_set(vm, VS_IN_CONDITIONS);
            }

            // cmds = stmts->statements[stmts->pos].commands;
            return vm_command_set(vm, VS_IN_CONDITIONS);
        }

        if (vm->status != EXIT_SUCCESS && (vm->state == VS_IN_IF_STATEMENTS || vm->state == VS_IN_CONDITIONS) && vm->cur_cmds->prev_op != OP_AND) {

            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }

        return vm_command_set(vm, VS_IN_IF_STATEMENTS);
    }

    if (vm->cur_stmt->type == LT_ELSE) {
        debug("in LT_ELSE");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_IF_STATEMENTS) {
            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }

        if (vm->state == VS_IN_ELIF_STATEMENTS) {
            vm->strs = NULL;
            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }

        if (vm->status == EXIT_SUCCESS && vm->state == VS_IN_ELSE_STATEMENTS) {
            return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
        }

        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS) {
            return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
        }

        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }

    if (vm->cur_stmt->type == LT_ELIF_CONDITIONS) {
        debug("in LT_ELIF_CONDITIONS");
        if (vm->state != VS_IN_CONDITIONS) {
            return vm_command_set(vm, VS_IN_CONDITIONS);
        }

        else if (vm->status != EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_AND) {
            if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                if (vm_next_else_statement(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }

                return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
            }

            vm->end = true;
            return EXIT_FAILURE_CONTINUE;
        }

        else if (vm->status != EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_OR) {
                if (vm->cur_stmt->type == LT_ELIF_CONDITIONS) {
                    return vm_command_set(vm, VS_IN_CONDITIONS);
                }
                else if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                    if (vm_next_else_statement(vm)) {
                        return EXIT_FAILURE_CONTINUE;
                    }
                    return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
                }
                else {
                    vm->end = true;
                    return EXIT_FAILURE_CONTINUE;
                }
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                if (vm_next_else_statement(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }
                return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
            }
            else if (vm->stmts->type == ST_IF_ELIF) {
                debug("going to next elif condition");
                if (vm_next_elif_condition(vm)) {
                    return EXIT_FAILURE_CONTINUE;
                }
                return vm_command_set(vm, VS_IN_CONDITIONS);
            }
            else {
                vm->end = true;
                return EXIT_FAILURE_CONTINUE;
            }
        }

        else if (vm->status == EXIT_SUCCESS && vm->cur_cmds->prev_op == OP_OR) {
            if (vm_next_elif_statement(vm)) {
                return EXIT_FAILURE_CONTINUE;
            }
            return vm_command_set(vm, VS_IN_ELIF_STATEMENTS);
        }

        else {
            return vm_command_set(vm, VS_IN_CONDITIONS);
        }
    }

    if (vm->cur_stmt->type == LT_ELIF) {
        debug("in LT_ELIF");
        debugf("vm->status %d\n", vm->status);

        if (vm->status == EXIT_SUCCESS && vm->state == VS_IN_CONDITIONS) {
            return vm_command_set(vm, VS_IN_ELIF_STATEMENTS);
        }

        else if (vm->status != EXIT_SUCCESS && vm->stmts->type == ST_IF_ELIF_ELSE) {
            if (vm_next_else_statement(vm)) {
                return EXIT_FAILURE_CONTINUE;
            }
            return vm_command_set(vm, VS_IN_ELSE_STATEMENTS);
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->cur_stmt->type == LT_ELIF_CONDITIONS) {
                if (!vm_next_elif_condition(vm)) {
                    return vm_command_set(vm, VS_IN_CONDITIONS);
                }
            }
        }
    }

    debug("vm_next_if: no conditions met, marking vm->end");
    debugf("vm->status %d\n", vm->status);
    vm->end = true;
    return EXIT_FAILURE_CONTINUE;
}

int vm_next_normal(Vm_Data* restrict vm)
{
    Commands* cmds = vm->cur_cmds;
    if (cmds && cmds->prev_op == OP_AND && vm->status != EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on AND.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        vm->end = true;
        return EXIT_SUCCESS;
    }
    else if (cmds && cmds->prev_op == OP_OR && vm->status == EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on OR.");
        vm->end = true;
        return EXIT_SUCCESS;
    }

    vm->strs = cmds->strs;
    vm->strs_n = cmds->count;
    int rv = vm_command_next(vm);
    if (rv) {
        return EXIT_FAILURE_CONTINUE;
    }

    if (!vm->end) {
        vm->op_current = cmds->prev_op;
    }

    return EXIT_SUCCESS;
}

int vm_next(Vm_Data* restrict vm)
{
    assert(vm); assert(vm->stmts);

    if (!vm->cur_cmds)
    {
        vm->end = true;
        return EXIT_FAILURE_CONTINUE;
    }

    switch (vm->stmts->type) {
    case ST_IF:
    case ST_IF_ELSE:
    case ST_IF_ELIF:
    case ST_IF_ELIF_ELSE: {
        debug("vm_next_if");
        return vm_next_if(vm);
    }
    default: {
        debug("vm_next_normal");
        return vm_next_normal(vm);
    }
    }
}

[[nodiscard]]
int vm_run_foreground(Statements* restrict stmts, Vm_Data* restrict vm, Arena* restrict scratch, pid_t shell_pgid)
{
    int pid = fork();
    if (pid < 0) {
        return vm_fork_failure(vm->command_position, stmts->pipes_count, &vm->pipes_io);
    }

    if (pid == 0) { // runs in the child process
        setpgid(0, 0);
        signal_reset();

        if (vm->op_current == OP_PIPE)
            pipe_connect(vm->command_position, stmts->pipes_count, &vm->pipes_io);

        char** buffers = estrtoarr(vm->strs, vm->strs_n, scratch);
        if (!buffers || !*buffers) {
            exit(-5);
        }
        execvp(*buffers, buffers);
        tty_perror("ncsh: Could not run command");
        exit(-1);
    }

    setpgid(pid, pid);
    tcsetpgrp(STDIN_FILENO, pid);

    if (vm->op_current == OP_PIPE) {
        pipe_stop(vm->command_position, stmts->pipes_count, &vm->pipes_io);
    }

    vm_waitpid(pid, vm);
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    return EXIT_SUCCESS;
}

void vm_check_background(Processes* restrict pcs)
{
    int s;
    pid_t pid;
    size_t j;
    while ((pid = waitpid(-1, &s, WNOHANG)) > 0) {
        for (j = 0; j < pcs->job_number; ++j) {
            if (pcs->pids[j] == pid)
                break;
        }
        if (WIFEXITED(s)) {
            tty_println("job [%zu] pid [%d]: exited with code %d", j + 1, pid, WEXITSTATUS(s));
            --pcs->job_number;
        }
        else if (WIFSIGNALED(s)) {
            tty_println("job [%zu] pid [%d]: killed by signal %d", j + 1, pid, WTERMSIG(s));
            --pcs->job_number;
        }
    }
}

[[nodiscard]]
int vm_run_background(Statements* restrict stmts, Vm_Data* restrict vm, Arena* restrict scratch, Processes* restrict pcs)
{
    int pid = fork();
    if (pid < 0) {
        return vm_fork_failure(vm->command_position, stmts->pipes_count, &vm->pipes_io);
    }

    if (pid == 0) { // runs in the child process
        setpgid(0, 0);
        signal_reset();

        char** buffers = estrtoarr(vm->strs, vm->strs_n, scratch);
        execvp(*buffers, buffers);
        if (!buffers || !*buffers) {
            exit(-5);
        }
        tty_perror("ncsh: Could not run command");
        exit(-1);
    }

    setpgid(pid, pid);
    size_t job_number = ++pcs->job_number;
    tty_println("job [%zu] pid [%d]", job_number, pid);
    pcs->pids[job_number - 1] = pid;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int vm_run(Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch)
{
    int rv;
    Vm_Data vm = {.stmts = stmts, .cur_stmt = stmts->head, .sh = shell, .s = scratch};
    vm.cur_cmds = vm.cur_stmt->commands;

    if (redirection_start_if_needed(&vm) != EXIT_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    // stmts->pos = 0;
    // Commands* cmds = stmts->statements[0].commands;
    // cmds->pos = 0;

    while (!vm.end && vm.cur_cmds) {
        if (vm_next(&vm)) {
            goto failure;
        }

        if (!vm.strs || !vm.strs[0].value) {
            rv = EXIT_FAILURE_CONTINUE;
            goto failure;
        }

        if (vm.op_current == OP_PIPE && !vm.end) {
            if (pipe_start(vm.command_position, &vm.pipes_io) != EXIT_SUCCESS) {
                rv = EXIT_FAILURE;
                goto failure;
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
            vm_math_process(&vm);
        }
        else if (stmts->is_bg_job) {
            rv = vm_run_background(stmts, &vm, scratch, &shell->pcs);
            if (rv != EXIT_SUCCESS) {
                goto failure;
            }
        }
        else {
            rv = vm_run_foreground(stmts, &vm, scratch, shell->pgid);
            if (rv != EXIT_SUCCESS) {
                goto failure;
            }
        }

        ++vm.command_position;
    }

    redirection_stop_if_needed(&vm);
    return vm_status_aggregate(&vm);

failure:
    redirection_stop_if_needed(&vm);
    return rv;
}

/* vm_execute
 * Executes the VM in interactive mode.
 */
[[nodiscard]]
int vm_execute(Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch)
{
    assert(shell); assert(stmts); assert(scratch);
    if (!stmts || !stmts->head) {
        return EXIT_SUCCESS;
    }

    if (shell->pcs.job_number > 0) {
        vm_check_background(&shell->pcs);
    }

    return vm_run(stmts, shell, scratch);
}

/* vm_execute_noninteractive
 * Executes the VM in noninteractive mode.
 * Please note that shell->arena is used for both perm & scratch arenas in noninteractive mode.
 */
[[nodiscard]]
int vm_execute_noninteractive(Statements* restrict stmts, Shell* restrict shell)
{
    assert(shell); assert(stmts);
    if (!stmts || !stmts->head) {
        return EXIT_SUCCESS;
    }

    return vm_run(stmts, shell, &shell->arena);
}
