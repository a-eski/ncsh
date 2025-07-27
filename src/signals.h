#pragma once

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include "defines.h"

extern jmp_buf env;
extern sig_atomic_t vm_child_pid;
extern volatile int sigwinch_caught;

/* Signal Handling */
static void signal_handler(int sig, siginfo_t* info, void* context)
{
    (void)context;

    if (sig == SIGWINCH) {
        sigwinch_caught = 1;
        return;
    }

    pid_t target = (pid_t)vm_child_pid;

    if (target != 0 && info->si_pid != target) { // kill child process with pid vm_child_pid
        if (!kill(target, sig)) {
            if (write(STDOUT_FILENO, "\n", 1) == -1) { // write is async/signal safe, do not use fflush, putchar, prinft
                perror("ncsh: Error writing to standard output while processing a signal");
                longjmp(env, FAILURE_SIG_HANDLER_WRITE);
            }
        }
        vm_child_pid = 0;
    }
    else { // jump to ncsh.c label exit to save history/autocompletions & z
        longjmp(env, FAILURE_SIG_HANDLER);
    }
}

[[nodiscard]]
static int signal_forward(int signum)
{
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(signum, &act, NULL)) {
        perror("ncsh: couldn't set up signal handler(s)");
        return errno;
    }

    return 0;
}

static void signal_init()
{
    signal(SIGPIPE, SIG_IGN); // Ignore sigpipe so it can be handled in code
    signal(SIGHUP, SIG_DFL);  // Stops the process if the terminal is closed
}
