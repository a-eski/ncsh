/* Copyright ncsh (C) by Alex Eski 2025 */
/* signals.h: signal handling, process group handling. */
/* as Stephen Bourne said, signal handling is the hard part of writing a shell. */

#pragma once

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

extern sig_atomic_t vm_child_pid;
extern volatile int sigwinch_caught;

static void signal_handler(int sig, [[maybe_unused]] siginfo_t* info, [[maybe_unused]] void* context)
{
    if (sig == SIGWINCH) {
        sigwinch_caught = 1;
        return;
    }

    if (sig == SIGINT || sig == SIGQUIT || sig == SIGTERM) {
        if (vm_child_pid != 0) {
            kill(vm_child_pid, sig);
        }
        return;
    }

    // TODO: consider using signals to handle bg jobs?
    /*if (sig == SIGCHLD) {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (pid == vm_child_pid) {
                vm_child_pid = 0;
            }
        }
        return;
    }*/
}

[[maybe_unused]]
[[nodiscard("Can return -1 or the shell's pgid, shell should call perror and then exit in the case of rv -1.")]]
static pid_t signal_init()
{
    struct sigaction sa_ncsh = {0};
    sa_ncsh.sa_sigaction = signal_handler;
    sa_ncsh.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&sa_ncsh.sa_mask);

    sigaction(SIGINT, &sa_ncsh, NULL);
    sigaction(SIGQUIT, &sa_ncsh, NULL);
    sigaction(SIGTERM, &sa_ncsh, NULL);
    sigaction(SIGCHLD, &sa_ncsh, NULL);
    sigaction(SIGWINCH, &sa_ncsh, NULL);

    struct sigaction sa_ign;
    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags = 0;

    sigaction(SIGPIPE, &sa_ign, NULL); // shell handles pipes
    sigaction(SIGTSTP, &sa_ign, NULL); // ignore these signals that can stop the shell
    sigaction(SIGTTIN, &sa_ign, NULL);
    sigaction(SIGTTOU, &sa_ign, NULL);

    pid_t shell_pid = getpid();
    pid_t shell_pgid = getpgrp();
    pid_t fg_pgid = tcgetpgrp(STDIN_FILENO);
    // If the shell is the login shell and already the foreground process,
    // do not call setpgid as it is an invalid operation in that case.
    if (shell_pgid == fg_pgid && getsid(0) == shell_pid) {
        return shell_pgid;
    }

    if (setpgid(shell_pid, shell_pid) < 0) {
        return -1;
    }

    if (tcsetpgrp(STDIN_FILENO, shell_pid) < 0) {
        return -1;
    }

    return shell_pid;
}

/* signal_reset
 * Reset signals to default behavior or ignore signals.
 * *** Only use in context of child process. ***
 * The shell needs to handle a variety of signals, but the child process should not, because applications may have their own signal handlers.
 */
[[maybe_unused]]
static void signal_reset()
{
    struct sigaction sa_dfl;
    sa_dfl.sa_handler = SIG_DFL;
    sigemptyset(&sa_dfl.sa_mask);
    sa_dfl.sa_flags = 0;

    sigaction(SIGINT,  &sa_dfl, NULL);
    sigaction(SIGQUIT, &sa_dfl, NULL);
    sigaction(SIGTERM, &sa_dfl, NULL);
    sigaction(SIGCHLD, &sa_dfl, NULL);
    sigaction(SIGWINCH, &sa_dfl, NULL);
    sigaction(SIGPIPE, &sa_dfl, NULL);

    struct sigaction sa_ign;
    sa_ign.sa_handler = SIG_IGN;
    sigemptyset(&sa_ign.sa_mask);
    sa_ign.sa_flags = 0;

    sigaction(SIGTSTP, &sa_ign, NULL);
    sigaction(SIGTTIN, &sa_ign, NULL);
    sigaction(SIGTTOU, &sa_ign, NULL);
}
