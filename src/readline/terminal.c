/* Copyright ncsh (C) by Alex Eski 2024 */
/* terminal.c: deal with the underlying tty */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "terminal.h"

#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

static struct termios otios;

Coordinates terminal_size(void)
{
    struct winsize window;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
    return (Coordinates){.x = window.ws_col, .y = window.ws_row};
}

Coordinates terminal_init(void)
{
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &otios) != 0) {
        perror(RED "ncsh: Could not get terminal settings" RESET);
        exit(EXIT_FAILURE);
    }

    // mouse support? investigate
    // printf("\x1b[?1049h\x1b[0m\x1b[2J\x1b[?1003h\x1b[?1015h\x1b[?1006h\x1b[?25l");

    struct termios tios = otios;
    tios.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tios) != 0) {
        perror(RED "ncsh: Could not set terminal settings" RESET);
    }

    return terminal_size();
}

void terminal_reset(void)
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &otios) != 0) {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

/*void terminal_move(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

void terminal_move_absolute(int x)
{
    printf("\033[%dG", x);
}*/

void terminal_move_right(int i)
{
    printf("\033[%dC", i);
}

void terminal_move_left(int i)
{
    printf("\033[%dD", i);
}

void terminal_move_up(int i)
{
    printf("\033[%dA", i);
}

void terminal_move_down(int i)
{
    printf("\033[%dB", i);
}

void terminal_move_to_end_of_previous_line()
{
    terminal_move_up(1);
    terminal_move_right(9999);
}

void terminal_move_to_start_of_next_line()
{
    if (write(STDOUT_FILENO, MOVE_CURSOR_NEXT_LINE, sizeof(MOVE_CURSOR_NEXT_LINE) - 1) == -1) {
        perror(RED NCSH_ERROR_STDOUT RESET);
    }
    fflush(stdout);
}

/*void terminal_line_insert(int i) {
    printf("\033[%dL", i);
}
void terminal_line_delete(int i) {
    printf("\033[%dM", i);
}

void terminal_characters_delete(int i)
{
    printf("\033[%dP", i);
}*/
