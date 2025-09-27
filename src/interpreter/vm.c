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
int vm_fork_failure(Vm_Data* restrict vm)
{
    if (vm->command_position != vm->stmts->pipes_count - 1) {
        if (vm->command_position % 2 != 0) {
            close(vm->pipes_io.fd_one[1]);
        }
        else {
            close(vm->pipes_io.fd_two[1]);
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
    enum Ops op = vm->ops[1];
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
    case OP_LESS_THAN_OR_EQUALS: {
        result = atoi(c1) <= atoi(c2);
        break;
    }
    case OP_GREATER_THAN: {
        result = atoi(c1) > atoi(c2);
        break;
    }
    case OP_GREATER_THAN_OR_EQUALS: {
        result = atoi(c1) >= atoi(c2);
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
Commands* vm_command_next(Commands* restrict cmds, Vm_Data* restrict vm)
{
    if (!cmds->next || !cmds->next->strs[0].value) {
        vm->cur_stmt = vm->cur_stmt->right;
        if (!vm->cur_stmt) {
            debugf("hit stmts->count %zu or stmts->cap %zu, marking vm end\n", stmts->count, stmts->cap);
            vm->end = true;
            return NULL;
        }
        else if (vm->state == VS_IN_IF_STATEMENTS && vm->cur_stmt->type != LT_IF) {
            debug("at end of if statements, marking vm end");
            vm->end = true;
            return NULL;
        }
        else {
            if (!vm->cur_stmt->commands) {
                debug("no commands after incrementing stmts->pos, marking vm end");
                vm->end = true;
                return NULL;
            }
            debugf("setting cmds to next statment command at pos %zu\n", stmts->pos);
            cmds = vm->cur_stmt->commands;
            cmds->pos = 0;
            return cmds;
        }
    }

    cmds = cmds->next;
    cmds->pos = 0;
    return cmds;
}

[[nodiscard]]
Commands* vm_command_set(Commands* restrict cmds, Vm_Data* restrict vm, enum Vm_State state)
{
    vm->strs = cmds->strs;
    vm->ops = cmds->ops;
    vm->strs_n = cmds->count;
    vm->state = state;
    vm->op_current = cmds->prev_op;
    return vm_command_next(cmds, vm);
}

Commands* vm_next_if_statement(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->right;
    if (vm->cur_stmt->type != LT_IF || !vm->cur_stmt->commands) {
        vm->end = true;
        return NULL;
    }

    return vm->cur_stmt->commands;
}

Statement* vm_find_else(Vm_Data* restrict vm)
{
    Statement* stmt = vm->stmts->head;
    while (stmt && stmt->type != LT_ELSE) {
        stmt = stmt->left;
    }
    return stmt;
}

Commands* vm_next_else_statement(Vm_Data* restrict vm)
{
    switch (vm->cur_stmt->type) {
    case LT_IF_CONDITIONS:
    case LT_ELIF_CONDITIONS:
        vm->cur_stmt = vm->cur_stmt->left;
        break;
    case LT_NORMAL:
    case LT_IF:
    case LT_ELIF:
        vm->cur_stmt = vm_find_else(vm);
        break;
    case LT_ELSE:
        vm->cur_stmt = vm->cur_stmt->prev->left;
        break;
    }
    if (!vm->cur_stmt || vm->cur_stmt->type != LT_ELSE || !vm->cur_stmt->commands) {
        vm->end = true;
        return NULL;
    }

    return vm->cur_stmt->commands;
}

Commands* vm_next_elif_condition(Vm_Data* restrict vm)
{
    if (!vm->cur_stmt->prev ||
        (vm->cur_stmt->type != LT_IF && vm->cur_stmt->prev->type == LT_IF_CONDITIONS) ||
        (vm->cur_stmt->type == LT_ELIF_CONDITIONS && vm->cur_stmt->prev->type == LT_ELIF_CONDITIONS)) {
        if (vm->cur_stmt->left && vm->cur_stmt->left->type == LT_ELIF_CONDITIONS) {
            vm->cur_stmt = vm->cur_stmt->left;
            return vm->cur_stmt->commands;
        }

        return NULL;
    }

    vm->cur_stmt = vm->cur_stmt->prev->left;
    if (!vm->cur_stmt || vm->cur_stmt->type != LT_ELIF_CONDITIONS ||
        !vm->cur_stmt->commands) {
        vm->end = true;
        return NULL;
    }

    return vm->cur_stmt->commands;
}

Commands* vm_next_elif_statement(Vm_Data* restrict vm)
{
    vm->cur_stmt = vm->cur_stmt->right;
    if (vm->cur_stmt->type != LT_ELIF || !vm->cur_stmt->commands) {
        vm->end = true;
        return NULL;
    }

    return vm->cur_stmt->commands;
}

[[nodiscard]]
Commands* vm_next_if(Commands* restrict cmds, Vm_Data* restrict vm)
{
    if (vm->cur_stmt->type == LT_IF_CONDITIONS) {
        if (vm->state != VS_IN_CONDITIONS) {
            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_AND) {
            if (vm->stmts->type == ST_IF_ELSE) {
                cmds = vm_next_else_statement(vm);
                if (!cmds) {
                    return NULL;
                }
                return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
            }
            else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                debug("going to next elif condition");
                cmds = vm_next_elif_condition(vm);
                if (!cmds) {
                    return NULL;
                }

                return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
            }
            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_OR) {
                if (cmds->ops[cmds->pos] == OP_TRUE || cmds->ops[cmds->pos] == OP_CONST) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }

                else if (vm->cur_stmt->type == LT_IF_CONDITIONS) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }

                else if (vm->stmts->type == ST_IF_ELSE) {
                    cmds = vm_next_else_statement(vm);
                    if (!cmds) {
                        return NULL;
                    }

                    return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
                }

                else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                    /*if (vm->cur_stmt->prev->type == LT_ELIF_CONDITIONS) {
                        cmds = vm_next_elif_condition(vm);
                        if (cmds) {
                            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                        }
                    }*/

                    debug("going to next elif condition");
                    cmds = vm_next_elif_condition(vm);
                    if (!cmds) {
                        return NULL;
                    }

                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }

                else {
                    vm->end = true;
                    return NULL;
                }
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->stmts->type == ST_IF_ELSE) {
                cmds = vm_next_else_statement(vm);
                if (!cmds) {
                    return NULL;
                }
                return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
            }

            else if (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE) {
                debug("going to next elif condition");
                cmds = vm_next_elif_condition(vm);
                if (!cmds) {
                    return NULL;
                }

                return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
            }

            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status == EXIT_SUCCESS && cmds->prev_op == OP_OR) {
            cmds = vm_next_if_statement(vm);
            if (!cmds) {
                return NULL;
            }

            return vm_command_set(cmds, vm, VS_IN_IF_STATEMENTS);
        }

        else {
            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
        }
    }

    if (vm->cur_stmt->type == LT_IF) {
        debug("in LT_IF");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS && vm->stmts->type == ST_IF_ELSE) {
            if (vm->cur_stmt->type != LT_ELSE) {
                cmds = vm_next_else_statement(vm);
                if (!cmds) {
                    return NULL;
                }
            }

            cmds = vm->cur_stmt->commands;
            return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
        }

        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS &&
                (vm->stmts->type == ST_IF_ELIF || vm->stmts->type == ST_IF_ELIF_ELSE)) {
            debug("in LT_IF ST_IF_ELIF or ST_IF_ELIF_ELSE");
            if (vm->cur_stmt->type != LT_ELIF_CONDITIONS) {
                cmds = vm_next_elif_condition(vm);
                if (!cmds) {
                    return NULL;
                }

                return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
            }

            cmds = vm->cur_stmt->commands;
            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
        }

        if (vm->status != EXIT_SUCCESS && (vm->state == VS_IN_IF_STATEMENTS || vm->state == VS_IN_CONDITIONS) && cmds->prev_op != OP_AND) {

            vm->end = true;
            return NULL;
        }

        return vm_command_set(cmds, vm, VS_IN_IF_STATEMENTS);
    }

    if (vm->cur_stmt->type == LT_ELSE) {
        debug("in LT_ELSE");
        debugf("vm->status %d\n", vm->status);
        if (vm->state == VS_IN_IF_STATEMENTS) {
            vm->end = true;
            return NULL;
        }

        if (vm->state == VS_IN_ELIF_STATEMENTS) {
            vm->strs = NULL;
            vm->end = true;
            return NULL;
        }

        if (vm->status == EXIT_SUCCESS && vm->state == VS_IN_ELSE_STATEMENTS) {
            return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
        }

        if (vm->state == VS_IN_CONDITIONS && vm->status != EXIT_SUCCESS) {
            return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
        }

        vm->end = true;
        return NULL;
    }

    if (vm->cur_stmt->type == LT_ELIF_CONDITIONS) {
        debug("in LT_ELIF_CONDITIONS");
        if (vm->state != VS_IN_CONDITIONS) {
            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_AND) {
            if (vm->cur_stmt->prev->type == LT_IF_CONDITIONS ||
                    vm->cur_stmt->prev->type == LT_ELIF_CONDITIONS) {
                cmds = vm_next_elif_condition(vm);
                if (cmds) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }
            }

            if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                cmds = vm_next_else_statement(vm);
                if (!cmds) {
                    return NULL;
                }

                return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
            }
            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status != EXIT_SUCCESS && cmds->prev_op == OP_OR) {
                if (vm->cur_stmt->type == LT_ELIF_CONDITIONS) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }
                else if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                    cmds = vm_next_else_statement(vm);
                    if (!cmds) {
                        return NULL;
                    }
                    return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
                }
                else {
                    vm->end = true;
                    return NULL;
                }
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->stmts->type == ST_IF_ELIF_ELSE) {
                cmds = vm_next_else_statement(vm);
                if (!cmds) {
                    return NULL;
                }
                return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
            }
            else if (vm->stmts->type == ST_IF_ELIF) {
                debug("going to next elif condition");
                cmds = vm_next_elif_condition(vm);
                if (!cmds) {
                    return NULL;
                }
                return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
            }
            else {
                vm->end = true;
                return NULL;
            }
        }

        else if (vm->status == EXIT_SUCCESS && cmds->prev_op == OP_OR) {
            cmds = vm_next_elif_statement(vm);
            if (!cmds) {
                return NULL;
            }
            return vm_command_set(cmds, vm, VS_IN_ELIF_STATEMENTS);
        }

        else {
            return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
        }
    }

    if (vm->cur_stmt->type == LT_ELIF) {
        debug("in LT_ELIF");
        debugf("vm->status %d\n", vm->status);

        if (vm->status == EXIT_SUCCESS && vm->state == VS_IN_CONDITIONS) {
            return vm_command_set(cmds, vm, VS_IN_ELIF_STATEMENTS);
        }

        else if (vm->status != EXIT_SUCCESS && vm->stmts->type == ST_IF_ELIF_ELSE) {
            if (vm->cur_stmt->prev->type == LT_ELIF_CONDITIONS) {
                cmds = vm_next_elif_condition(vm);
                if (cmds) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }
            }

            cmds = vm_next_else_statement(vm);
            if (!cmds) {
                return NULL;
            }
            return vm_command_set(cmds, vm, VS_IN_ELSE_STATEMENTS);
        }

        else if (vm->status != EXIT_SUCCESS) {
            if (vm->cur_stmt->prev->type == LT_ELIF_CONDITIONS) {
                cmds = vm_next_elif_condition(vm);
                if (cmds) {
                    return vm_command_set(cmds, vm, VS_IN_CONDITIONS);
                }
            }
        }
    }

    debug("vm_next_if: no conditions met, marking vm->end");
    debugf("vm->status %d\n", vm->status);
    vm->end = true;
    return NULL;
}

Commands* vm_next_normal(Vm_Data* restrict vm)
{
    if (vm->cur_cmds && vm->cur_cmds->prev_op == OP_AND && vm->status != EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on AND.");
        vm->status = EXIT_FAILURE_CONTINUE; // make sure condition failure doesn't cause shell to exit
        vm->end = true;
        return NULL;
    }
    else if (vm->cur_cmds && vm->cur_cmds->prev_op == OP_OR && vm->status == EXIT_SUCCESS) {
        debug("breaking out of VM loop, short circuiting on OR.");
        vm->end = true;
        return NULL;
    }

    vm->strs = vm->cur_cmds->strs;
    vm->ops = vm->cur_cmds->ops;
    vm->strs_n = vm->cur_cmds->count;
    vm->ops = vm->cur_cmds->ops;
    vm->cur_cmds = vm_command_next(vm->cur_cmds, vm);

    if (!vm->end) {
        vm->op_current = vm->cur_cmds->prev_op;
    }

    return vm->cur_cmds;
}

Commands* vm_next(Commands* cmds, Vm_Data* restrict vm)
{
    assert(vm); assert(vm->stmts);

    if (!cmds)
    {
        vm->end = true;
        return NULL;
    }

    switch (vm->stmts->type) {
    case ST_IF:
    case ST_IF_ELSE:
    case ST_IF_ELIF:
    case ST_IF_ELIF_ELSE: {
        debug("vm_next_if");
        return vm_next_if(cmds, vm);
    }
    default: {
        debug("vm_next_normal");
        return vm_next_normal(vm);
    }
    }
}

[[nodiscard]]
int vm_run_foreground(Vm_Data* restrict vm, pid_t shell_pgid)
{
    int pid = fork();
    if (pid < 0) {
        return vm_fork_failure(vm);
    }

    if (pid == 0) { // runs in the child process
        setpgid(0, 0);
        signal_reset();

        if (vm->op_current == OP_PIPE)
            pipe_connect(vm->command_position, vm->stmts->pipes_count, &vm->pipes_io);

        char** buffers = estrtoarr(vm->strs, vm->strs_n, vm->s);
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
        pipe_stop(vm->command_position, vm->stmts->pipes_count, &vm->pipes_io);
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
int vm_run_background(Vm_Data* restrict vm, Processes* restrict pcs)
{
    int pid = fork();
    if (pid < 0) {
        return vm_fork_failure(vm);
    }

    if (pid == 0) { // runs in the child process
        setpgid(0, 0);
        signal_reset();

        char** buffers = estrtoarr(vm->strs, vm->strs_n, vm->s);
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
    vm.cur_cmds->pos = 0;

    if (redirection_start_if_needed(&vm) != EXIT_SUCCESS) {
        return EXIT_FAILURE_CONTINUE;
    }

    while (!vm.end && vm.cur_cmds) {
        vm.cur_cmds = vm_next(vm.cur_cmds, &vm);

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
            rv = vm_run_background(&vm, &shell->pcs);
            if (rv != EXIT_SUCCESS) {
                goto failure;
            }
        }
        else {
            rv = vm_run_foreground(&vm, shell->pgid);
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
    if (!stmts || !stmts->head || !stmts->head->commands) {
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
