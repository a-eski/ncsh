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
#include "ncsh_terminal.h"
#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

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

void ncsh_terminal_reset(void)
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_os) != 0) {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

void ncsh_terminal_init(void)
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

struct ncsh_Coordinates ncsh_terminal_size_get(void)
{
    return (struct ncsh_Coordinates){.x = window.ws_col, .y = window.ws_row};
}

int ncsh_terminal_size_x(void) {
    return window.ws_col;
}

int ncsh_terminal_size_y(void) {
    return window.ws_row;
}

void ncsh_terminal_move(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

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

size_t ncsh_terminal_prompt_size(size_t user_len, size_t dir_len)
{
    // shell prompt format:
    // {user} {directory} {symbol} {buffer}
    // user, directory include null termination, use as space for len
    //     {user}{space (\0)}      {directory}{space (\0)}     {>}  {space}     {buffer}
    return user_len + dir_len + 1 + 1;
}

struct ncsh_Coordinates ncsh_terminal_position(void)
{
    char buffer[T_BUFFER_LENGTH] = {0};
    int i = 0;
    int power = 0;
    char character = 0;
    struct ncsh_Coordinates cursor_position = {0};

    if (write(STDOUT_FILENO, GET_CURSOR_POSITION, sizeof(GET_CURSOR_POSITION) - 1) == -1) {
        perror(RED "ncsh: Error writing to stdout" RESET);
        fflush(stdout);
        return cursor_position;
    }

    for (i = 0; i < T_BUFFER_LENGTH && character != TERMINAL_RETURN; ++i) {
        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED "ncsh: Could not get cursor position" RESET);
            return cursor_position;
        }
        buffer[i] = character;
    }

    if (i < 2 || i == T_BUFFER_LENGTH - 1) {
        return cursor_position;
    }

    for (i -= 2, power = 1; buffer[i] != ';'; i--, power *= 10) {
        cursor_position.x += (buffer[i] - '0') * power;
    }

    for (i--, power = 1; buffer[i] != '['; i--, power *= 10) {
        cursor_position.y += (buffer[i] - '0') * power;
    }

    return cursor_position;
}
