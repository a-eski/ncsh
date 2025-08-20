// jobctl.c - minimal interactive shell with job control

#define _POSIX_C_SOURCE 200809L

// #include <string.h>
// #include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

static pid_t shell_pgid;
static struct termios shell_tmodes;

static void init_shell(void) {
    // Must be interactive
    if (!isatty(STDIN_FILENO)) return;

    // Ignore signals that stop shells
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    shell_pgid = getpid();

    // Put shell in its own process group
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        perror("setpgid(shell)");
        exit(1);
    }

    // Grab control of the terminal
    if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
        perror("tcsetpgrp(shell)");
        exit(1);
    }

    // Save terminal modes
    tcgetattr(STDIN_FILENO, &shell_tmodes);
}

static void run_foreground(char *prog, char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }

    if (pid == 0) {
        // --- Child ---
        pid_t cpid = getpid();
        setpgid(cpid, cpid);          // put child in new process group
        tcsetpgrp(STDIN_FILENO, cpid);// take terminal

        // Restore default signals
        signal(SIGINT,  SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        execvp(prog, argv);
        perror("execvp");
        _exit(127);
    }

    // --- Parent (shell) ---
    setpgid(pid, pid);                // make sure child has its own pgrp
    tcsetpgrp(STDIN_FILENO, pid);     // hand terminal to child

    int status;
    waitpid(pid, &status, WUNTRACED); // wait for it

    // Take terminal back
    tcsetpgrp(STDIN_FILENO, shell_pgid);
    tcgetattr(STDIN_FILENO, &shell_tmodes); // re-grab terminal modes
}

int main(void) {
    init_shell();

    char* buf[3] = {
        [0] = "gdb",
        [1] = "./../bin/ncsh",
        [2] = NULL
    };

    run_foreground(buf[0], buf);

    /*printf("Mini shell: type a program name (or 'exit')\n");

    char buf[256];
    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) break;
        if (buf[0] == '\n') continue;
        buf[strcspn(buf, "\n")] = 0;
        if (!*buf) continue;
        if (!strcmp(buf, "exit")) break;

        char *argv[] = { buf, NULL };
        run_foreground(buf, argv);
    }*/
}
