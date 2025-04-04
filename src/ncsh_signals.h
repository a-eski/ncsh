/* Copyright (C) ncsh by Alex Eski 2025 */

#pragma once

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_global.h"

sig_atomic_t vm_child_pid;

static void ncsh_signal_handler(int signum, siginfo_t* const info, void* const context)
{
    (void)context; // to prevent compiler warnings
    const __pid_t target = vm_child_pid;

    if (target != 0 && info->si_pid != target) { // kill child process with pid vm_child_pid
        if (!kill(target, signum)) {
            if (write(STDOUT_FILENO, "\n", 1) == -1) { // write is async/signal safe, do not use fflush, putchar, prinft
                perror(RED "ncsh: Error writing to standard output while processing a signal" RESET);
            }
        }
        vm_child_pid = 0;
    }
    else { // jump to ncsh.c label exit to save history/autocompletions & z
        longjmp(env, 1);
    }
}

static int ncsh_signal_forward(const int signum)
{
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = ncsh_signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(signum, &act, NULL)) {
        return errno;
    }

    return 0;
}
