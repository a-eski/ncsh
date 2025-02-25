// Copyright (c) ncsh by Alex Eski 2024

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
// #if defined(HAVE_TERMIOS_H)
#include <termios.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_defines.h"
#include "ncsh_terminal.h"
#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

/* Static Variables */
static struct termios original_terminal_settings;

/* Signal Handling */
/*static void ncsh_terminal_signal_handler(int signum)
{
    if (signum != SIGWINCH)
        return;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    if (write(STDOUT_FILENO, "ncsh window change handled\n", sizeof("ncsh window change handled\n")) == -1)
        perror("sighandler error");
}*/

void ncsh_terminal_reset(void)
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_settings) != 0) {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

struct ncsh_Coordinates ncsh_terminal_init(void)
{
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);

    if (tcgetattr(STDIN_FILENO, &original_terminal_settings) != 0) {
        perror(RED "ncsh: Could not get terminal settings" RESET);
        exit(EXIT_FAILURE);
    }

    // mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios terminal_settings = original_terminal_settings;
    terminal_settings.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    terminal_settings.c_cc[VMIN] = 1;
    terminal_settings.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settings) != 0) {
        perror(RED "ncsh: Could not set terminal settings" RESET);
    }

    signal(SIGHUP, SIG_DFL); // Stops the process if the terminal is closed
    // signal(SIGWINCH, ncsh_terminal_signal_handler); // Sets window size when window size changed

    return (struct ncsh_Coordinates){.x = window.ws_col, .y = window.ws_row};
}

/*void ncsh_terminal_move(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

void ncsh_terminal_move_absolute(int x)
{
    printf("\033[%dG", x);
}*/

void ncsh_terminal_move_right(int i)
{
    printf("\033[%dC", i);
}

void ncsh_terminal_move_left(int i)
{
    printf("\033[%dD", i);
}

void ncsh_terminal_move_up(int i)
{
    printf("\033[%dA", i);
}

void ncsh_terminal_move_down(int i)
{
    printf("\033[%dB", i);
}

void ncsh_terminal_move_to_end_of_previous_line()
{
    ncsh_terminal_move_up(1);
    ncsh_terminal_move_right(9999);
}

void ncsh_terminal_move_to_start_of_next_line()
{
    if (write(STDOUT_FILENO, MOVE_CURSOR_NEXT_LINE, sizeof(MOVE_CURSOR_NEXT_LINE) - 1) == -1) {
        perror(RED NCSH_ERROR_STDOUT RESET);
    }
    fflush(stdout);
}

/*void ncsh_terminal_line_insert(int i) {
    printf("\033[%dL", i);
}
void ncsh_terminal_line_delete(int i) {
    printf("\033[%dM", i);
}*/

void ncsh_terminal_characters_delete(int i) {
    printf("\033[%dP", i);
}
