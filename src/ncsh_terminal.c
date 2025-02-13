// Copyright (c) ncsh by Alex Eski 2024

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh_platform.h"
#include "ncsh_terminal.h"
#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

void ncsh_terminal_os_reset(void)
{
   ncsh_platform_terminal_reset();
}

void ncsh_terminal_os_init(void)
{
    ncsh_platform_terminal_init();
}

struct ncsh_Coordinates ncsh_terminal_size_get(void)
{
    return ncsh_platform_terminal_size_get();
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

enum eskilib_Result ncsh_terminal_init(struct ncsh_Terminal* terminal)
{
    assert(terminal);

    terminal->reprint_prompt = true; // print for first entry into readline.
    terminal->user.value = getenv("USER");
    terminal->user.length = strlen(terminal->user.value) + 1;
    terminal->size = ncsh_terminal_size_get();

    return E_SUCCESS;
}

void ncsh_terminal_exit(struct ncsh_Terminal *terminal)
{
    (void)terminal;
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
