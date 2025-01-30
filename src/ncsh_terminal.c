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
#include "eskilib/eskilib_result.h"
#include "ncsh_defines.h"
#include "ncsh_terminal.h"
#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

/* Static Variables */
static struct termios terminal_os;
static struct termios original_terminal_os;
static struct winsize window;

/* Signal Handling */
/*static void ncsh_terminal_signal_handler(int signum)
{
    if (signum != SIGWINCH)
        return;

    ncsh_terminal_size_set();
    if (write(STDOUT_FILENO, "ncsh window change handled\n", sizeof("ncsh window change handled\n")) == -1)
        perror("sighandler error");
}*/

void ncsh_terminal_os_reset(void)
{
    fflush(stdout);
    if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_os) != 0) {
        perror(RED "ncsh: Could not restore terminal settings" RESET);
    }
}

void ncsh_terminal_os_init(void)
{
    if (!isatty(STDIN_FILENO)) {
        fprintf(stderr, "Not running in a terminal.\n");
        exit(EXIT_FAILURE);
    }

    if (tcgetattr(STDIN_FILENO, &original_terminal_os) != 0) {
        perror(RED "ncsh: Could not get terminal settings" RESET);
        exit(EXIT_FAILURE);
    }

    terminal_os = original_terminal_os;
    terminal_os.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    terminal_os.c_cc[VMIN] = 1;
    terminal_os.c_cc[VTIME] = 0;
    // terminal.c_cc[VEOF] = CTRL_D;
    // terminal.c_cc[VINTR] = CTRL_C;
    // terminal.c_cc[VKILL] = CTRL_U;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_os) != 0) {
        perror(RED "ncsh: Could not set terminal settings" RESET);
    }

    signal(SIGHUP, SIG_DFL); // Stops the process if the terminal is closed
    // signal(SIGWINCH, ncsh_terminal_signal_handler); // Sets window size when window size changed
}

void ncsh_terminal_size_set(void)
{
    // todo: look at getenv("LINES"), getenv("COLUMNS") since ioctl is not portable
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &window);
}

struct ncsh_Coordinates ncsh_terminal_size_get(void)
{
    return (struct ncsh_Coordinates){.x = window.ws_col, .y = window.ws_row};
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

enum eskilib_Result ncsh_terminal_malloc(struct ncsh_Terminal* terminal)
{
    assert(terminal);

    terminal->prompt.user.value = getenv("USER");
    terminal->prompt.user.length = strlen(terminal->prompt.user.value) + 1;
    terminal->prompt.directory.length = 0;
    terminal->prompt.directory.value = malloc(PATH_MAX);
    if (!terminal->prompt.directory.value) {
	return E_FAILURE_MALLOC;
    }

    ncsh_terminal_size_set();
    terminal->size = ncsh_terminal_size_get();

    return E_SUCCESS;
}

void ncsh_terminal_init(struct ncsh_Terminal* terminal)
{
    ncsh_terminal_size_set();
    terminal->size = ncsh_terminal_size_get();
}

void ncsh_terminal_free(struct ncsh_Terminal *terminal)
{
    free(terminal->prompt.directory.value);
}

int ncsh_terminal_prompt_size(struct ncsh_Prompt* prompt)
{
    // shell prompt format:
    // {user} {directory} {symbol} {buffer}
    // user, directory include null termination, use as space for len
    //     {user}{space (\0)}    {directory}{space (\0)}    {>} {space}     {buffer}
    return prompt->user.length + prompt->directory.length + 1 + 1;
}

void ncsh_terminal_line_size(size_t buf_pos, struct ncsh_Terminal* terminal)
{
    assert(buf_pos <= INT_MAX);
    assert(terminal);
    assert(terminal->size.x);

    int new_size = terminal->lines.y == 1
        ? ncsh_terminal_prompt_size(&terminal->prompt) + (int)buf_pos
        : (int)buf_pos;
    if (new_size > terminal->size.x)
    {
        ++terminal->lines.y;
    }

    terminal->lines.x = new_size;
}

void ncsh_terminal_line_new(struct ncsh_Terminal* terminal)
{
    assert(terminal);

    terminal->lines.y = 1;
    terminal->lines.x = ncsh_terminal_prompt_size(&terminal->prompt);
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

/* Prompt */
#ifdef NCSH_SHORT_DIRECTORY
size_t ncsh_terminal_prompt_short_directory(char* cwd, char* output)
{
    uint_fast32_t i = 1;
    uint_fast32_t last_slash_pos = 0;
    uint_fast32_t second_to_last_slash = 0;

    while (cwd[i] != '\n' && cwd[i] != '\0') {
        if (cwd[i] == '/') {
            second_to_last_slash = last_slash_pos;
            last_slash_pos = i + 1;
        }
        ++i;
    }

    memcpy(output, cwd + second_to_last_slash - 1, i - second_to_last_slash + 1);
    output[i - second_to_last_slash + 1] = '\0';

    size_t result_len = i - second_to_last_slash + 2; // null termination included in len
    assert(result_len == strlen(cwd + second_to_last_slash - 1) + 1);

    return result_len;
}
#endif /* ifdef NCSH_SHORT_DIRECTORY */

eskilib_nodiscard int_fast32_t ncsh_terminal_prompt(struct ncsh_Terminal* terminal)
{
    if (!getcwd(terminal->prompt.cwd, sizeof(terminal->prompt.cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

#ifdef NCSH_SHORT_DIRECTORY
    terminal->prompt.directory.length = ncsh_terminal_prompt_short_directory(terminal->prompt.cwd, terminal->prompt.directory.value);
    printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", terminal->prompt.user.value, terminal->prompt.directory.value);
#else
    strcpy(terminal->prompt.directory.value, terminal->prompt.cwd);
    terminal->prompt.directory.length = strlen(terminal->prompt.directory.value) + 1;
    printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", terminal->prompt.user.value, terminal->prompt.directory.value);
#endif /* ifdef NCSH_SHORT_DIRECTORY */

    ncsh_terminal_line_new(terminal);

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);

    return EXIT_SUCCESS;
}
