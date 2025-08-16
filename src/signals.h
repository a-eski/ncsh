#pragma once

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

extern sig_atomic_t vm_child_pid;
extern volatile int sigwinch_caught;
extern volatile int sigint_caught;

static void signal_handler(int sig, [[maybe_unused]] siginfo_t* info, [[maybe_unused]] void* context)
{
    if (sig == SIGWINCH) {
        sigwinch_caught = 1;
        return;
    }

    if (sig == SIGINT || sig == SIGQUIT) {
        if (vm_child_pid != 0) {
            kill(vm_child_pid, sig);
        }
        else {
            sigint_caught = 1;
        }
        return;
    }

    if (sig == SIGCHLD) {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            if (pid == vm_child_pid) {
                vm_child_pid = 0;
            }
        }
        return;
    }
}

static void signal_init()
{
    struct sigaction act = {0};
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGWINCH, &act, NULL);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_DFL);
}
