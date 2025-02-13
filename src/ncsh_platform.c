#include <string.h>

#include "ncsh_platform.h"

#if defined(linux) || defined(__unix__)

#include <glob.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <bits/types/siginfo_t.h>

int ncsh_platform_mkdir(const char* dir)
{
    return mkdir(dir, 0755);
}

int ncsh_platform_kill(pid_t pid)
{
    return kill(pid, SIGTERM);
}

void ncsh_platform_glob(const char* buffer, struct ncsh_Args* args)
{
    glob_t glob_buf = {0};
    size_t glob_len;
    glob(buffer, GLOB_DOOFFS, NULL, &glob_buf);

    for (size_t i = 0; i < glob_buf.gl_pathc; ++i) {
        glob_len = strlen(glob_buf.gl_pathv[i]) + 1;
        if (!glob_len || glob_len >= NCSH_MAX_INPUT) {
            break;
        }
        buf_pos = glob_len;
        args->values[args->count] = malloc(buf_pos);
        memcpy(args->values[args->count], glob_buf.gl_pathv[i], glob_len);
        args->ops[args->count] = OP_CONSTANT;
        args->lengths[args->count] = buf_pos;
        ++args->count;
        if (args->count >= NCSH_PARSER_TOKENS - 1) {
            break;
        }
    }

    globfree(&glob_buf);
}

/* Static Variables */
static struct termios original_terminal_os;
static struct winsize window;

/* Signal Handling */
/*static void ncsh_terminal_signal_handler(int signum)
{
    if (signum != SIGWINCH)
        return;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    if (write(STDOUT_FILENO, "ncsh window change handled\n", sizeof("ncsh window change handled\n")) == -1)
        perror("sighandler error");
}*/

void ncsh_platform_terminal_init()
{
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    if (tcgetattr(STDIN_FILENO, &original_terminal_os) != 0) {
        perror(RED "ncsh: Could not get terminal settings" RESET);
        exit(EXIT_FAILURE);
    }

    // mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios terminal_os = original_terminal_os;
    terminal_os.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    terminal_os.c_cc[VMIN] = 1;
    terminal_os.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_os) != 0) {
        perror(RED "ncsh: Could not set terminal settings" RESET);
    }

    signal(SIGHUP, SIG_DFL); // Stops the process if the terminal is closed
    // signal(SIGWINCH, ncsh_terminal_signal_handler); // Sets window size when window size changed
}

void ncsh_platform_terminal_reset()
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_os) != 0) {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

struct ncsh_Coordinates ncsh_platform_terminal_size_get(void)
{
    return (struct ncsh_Coordinates){.x = window.ws_col, .y = window.ws_row};
}


/* Signal Handling */
static _Atomic pid_t ncsh_vm_atomic_internal_child_pid = 0;

static inline void ncsh_vm_atomic_child_pid_set(pid_t pid)
{
    ncsh_vm_atomic_internal_child_pid = pid;
}

static inline pid_t ncsh_vm_atomic_child_pid_get(void)
{
    return ncsh_vm_atomic_internal_child_pid;
}

#else

int ncsh_platform_mkdir(const char* dir)
{
    return mkdir(dir);
}

int ncsh_platform_kill(pid_t pid)
{
    (void)pid;
    return 0;
}

void ncsh_platform_glob(const char* buffer, struct ncsh_Args* args)
{
    (void)buffer;
    (void)args;
}

void ncsh_platform_terminal_init() {}
void ncsh_platform_terminal_reset() {}

struct ncsh_Coordinates ncsh_platform_terminal_size_get(void)
{
    return (struct ncsh_Coordinates){0};
}

#endif
