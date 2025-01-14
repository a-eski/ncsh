// Copyright (c) ncsh by Alex Eski 2024

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
// #include <uv.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_terminal.h"
#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

// #if defined(HAVE_TERMIOS_H)
static struct termios terminal;
static struct termios original_terminal;

void ncsh_terminal_reset(void)
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal) != 0)
    {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

void ncsh_terminal_init(void)
{
    // if (uv_guess_handle(fileno(stream)) != UV_TTY)
    if (!isatty(STDIN_FILENO))
    {
        fprintf(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &original_terminal) != 0)
    {
        perror(RED "ncsh: Could not get terminal settings" RESET);
        exit(EXIT_FAILURE);
    }

    terminal = original_terminal;
    terminal.c_lflag &= (tcflag_t)~(ICANON | ECHO);
    terminal.c_cc[VMIN] = 1;
    terminal.c_cc[VTIME] = 0;
    // terminal.c_cc[VEOF] = CTRL_D;
    // terminal.c_cc[VINTR] = CTRL_C;
    // terminal.c_cc[VKILL] = CTRL_U;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) != 0)
    {
        perror(RED "ncsh: Could not set terminal settings" RESET);
    }

    signal(SIGHUP, SIG_DFL); // Stops the process if the terminal is closed
}

struct ncsh_Coordinates ncsh_terminal_size(void)
{
    // todo: look at getenv("LINES"), getenv("COLUMNS") since ioctl is not portable
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return (struct ncsh_Coordinates){.x = w.ws_col, .y = w.ws_row};
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

struct ncsh_Coordinates ncsh_terminal_position(void)
{
    char buffer[T_BUFFER_LENGTH] = {0};
    int i = 0;
    int power = 0;
    char character = 0;
    struct ncsh_Coordinates cursor_position = {0};

    if (write(STDOUT_FILENO, GET_CURSOR_POSITION, GET_CURSOR_POSITION_LENGTH) == -1)
    {
        perror(RED "ncsh: Error writing to stdout" RESET);
        fflush(stdout);
        return cursor_position;
    }

    for (i = 0; i < T_BUFFER_LENGTH && character != TERMINAL_RETURN; ++i)
    {
        if (read(STDIN_FILENO, &character, 1) == -1)
        {
            perror(RED "ncsh: Could not get cursor position" RESET);
            return cursor_position;
        }
        buffer[i] = character;
    }

    if (i < 2 || i == T_BUFFER_LENGTH - 1)
    {
        return cursor_position;
    }

    for (i -= 2, power = 1; buffer[i] != ';'; i--, power *= 10)
        cursor_position.x += (buffer[i] - '0') * power;

    for (i--, power = 1; buffer[i] != '['; i--, power *= 10)
        cursor_position.y += (buffer[i] - '0') * power;

    return cursor_position;
}
