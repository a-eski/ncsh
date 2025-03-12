// Copyright (c) ncsh by Alex Eski 2024

/* from VM */
/*if (setpgid(pid, pid) == 0) {
                    perror(RED "ncsh: Error setting up process group ID for child process" RESET);
                }*/


/*if (ncsh_vm_signal_forward(SIGINT) ||
                ncsh_vm_signal_forward(SIGHUP) ||
                ncsh_vm_signal_forward(SIGTERM) ||
                ncsh_vm_signal_forward(SIGQUIT) ||
                ncsh_vm_signal_forward(SIGUSR1) ||
                ncsh_vm_signal_forward(SIGUSR2)) {
                perror("ncsh: Error setting up signal handlers");
                return NCSH_COMMAND_EXIT_FAILURE;
            }*/

/* from readline */
/* Need to figure out how to handle SIGWINCH signal */

// #define _POSIX_C_SOURCE 200809L
// #include <signal.h>
/* Signals */
/*static _Atomic bool ncsh_screen_size_changed = false;

static inline void ncsh_screen_size_changed_set(bool value)
{
    ncsh_screen_size_changed = value;
}

static inline bool ncsh_screen_size_changed_get()
{
    return ncsh_screen_size_changed;
}

// int signum, siginfo_t *info, void *context
// SIGWINCH handler for terminal resize
static void ncsh_sigwinch_handler(int signum)
{
    (void)signum;
    ncsh_screen_size_changed_set(true);
}

static int ncsh_screen_size_change_handler()
{
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_handler = ncsh_sigwinch_handler;
    act.sa_flags = SIGWINCH;
    if (sigaction(SIGWINCH, &act, NULL))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}*/

// in ncsh_init
/*if (ncsh_screen_size_change_handler())
    {
        perror(RED "ncsh: error setting up SIGWINCH signal handler" RESET);
        ncsh_exit(shell);
        return EXIT_FAILURE;
    }*/

// in ncsh_readline at end of loop
/*if (ncsh_screen_size_changed_get())
        {
                ncsh_screen_size_changed_set(false);
            ncsh_terminal_size_set();
            shell->terminal_size = ncsh_terminal_size_get();
            printf("Setting screen size after SIGWINCH %d, %d\n", shell->terminal_size.x, shell->terminal_size.y);
        }*/

