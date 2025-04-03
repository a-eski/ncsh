/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "../eskilib/eskilib_colors.h"
#include "../ncsh_defines.h"
#include "ncsh_terminal.h"

/*void ncsh_terminal_move(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

void ncsh_terminal_move_absolute(int x)
{
    printf("\033[%dG", x);
}*/

void ncsh_terminal_move_right(const int i)
{
    printf("\033[%dC", i);
}

void ncsh_terminal_move_left(const int i)
{
    printf("\033[%dD", i);
}

void ncsh_terminal_move_up(const int i)
{
    printf("\033[%dA", i);
}

void ncsh_terminal_move_down(const int i)
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
}

void ncsh_terminal_characters_delete(int i) {
    printf("\033[%dP", i);
}*/
