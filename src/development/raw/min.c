// raw experiment

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <unistd.h>

#include "../../ncsh_terminal.h"

#define LINE_LIMIT 10

struct screen {
    int max_x;
    int max_y;
    int line_total_x; // buffer_len
    bool reprint_prompt;
    char* prompt;
    int prompt_len;
    char* buffer;
};

[[nodiscard]]
int screen_init(struct screen* screen)
{
    struct ncsh_Coordinates terminal_size = ncsh_terminal_init();

    screen->max_x = terminal_size.x;
    screen->max_y = terminal_size.y;
    screen->reprint_prompt = true;
    screen->prompt = "> ";
    screen->prompt_len = sizeof("> ") - 1;

    screen->buffer = calloc(PATH_MAX, sizeof(char));
    if (!screen->buffer) {
	return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void screen_exit(struct screen* screen)
{
    free(screen->buffer);

    ncsh_terminal_reset();
}

int main()
{
    char new_char = 0;
    struct screen screen = {0};
    if (screen_init(&screen) != EXIT_SUCCESS) {
	return EXIT_FAILURE;
    }

    while (1) {
        if (read(STDIN_FILENO, &new_char, 1) == -1) {
	    goto exit;
        }
        screen.buffer[screen.line_total_x++] = new_char;
	putchar(new_char);
        fflush(stdout);

        switch (new_char) {
            case 4: {
                goto exit;
            }
            case '\r':
            case '\n': {
                continue;
            }
        }
    }

exit:
    fflush(stdout);
    screen_exit(&screen);

    return EXIT_SUCCESS;
}
