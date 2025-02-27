// raw experiment

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "../../ncsh_terminal.h"
#include "../../ncsh_defines.h"

#define LINE_LIMIT 10

struct screen {
    int y;
    int x;
    int max_x;
    int max_y;
    int line_start_y;
    int line_total_x; // buffer_len
    int lines_x[LINE_LIMIT];
    int lines_y;
    bool reprint_prompt;
    char* prompt;
    int prompt_len;
    char* buffer;
};

[[nodiscard]]
int screen_init(struct screen* screen)
{
    ncsh_terminal_init();

    struct ncsh_Coordinates terminal_size = ncsh_terminal_size_get();
    screen->max_x = terminal_size.x;
    screen->max_y = terminal_size.y;
    screen->reprint_prompt = true;
    screen->prompt = "> ";
    screen->prompt_len = sizeof("> ") - 1;

    screen->buffer = calloc(PATH_MAX, sizeof(char));
    if (!screen->buffer) {
	return EXIT_FAILURE;
    }

    screen->x = screen->prompt_len;

    return EXIT_SUCCESS;
}

void screen_exit(struct screen* screen)
{
    free(screen->buffer);

    ncsh_terminal_reset();
}

void screen_prompt(struct screen* screen)
{
    screen->x = 0;
    printf("> ");
    ++screen->y;
    screen->x = screen->prompt_len;
    screen->line_start_y = screen->y;
    screen->line_total_x = 0;
    screen->lines_y = 0;
    memset(screen->buffer, 0, sizeof(char) * PATH_MAX);
    screen->reprint_prompt = false;
}

void screen_update(struct screen* screen)
{
    if (screen->x >= screen->max_x) {
	screen->lines_x[screen->lines_y] = screen->x;
        ++screen->y;
        ++screen->lines_y;
	screen->x = 0;
        putchar('\n');
        fflush(stdout);
    }
}

void screen_save(struct screen* screen) {
    printf("\n%s\n", screen->buffer);
    screen->lines_x[screen->lines_y] = screen->x;
    screen->reprint_prompt = true;
}

int main()
{
    char new_char = 0;
    struct screen screen = {0};
    if (screen_init(&screen) != EXIT_SUCCESS) {
	return EXIT_FAILURE;
    }

    while (1) {
    	if (screen.reprint_prompt) {
	    screen_prompt(&screen);
    	    fflush(stdout);
    	}

        if (read(STDIN_FILENO, &new_char, 1) == -1) {
	    goto exit;
        }
        screen.buffer[screen.line_total_x++] = new_char;
	putchar(new_char);
        ncsh_write_literal(ERASE_CURRENT_LINE)
        fflush(stdout);
	++screen.x;

        switch (new_char) {
            case CTRL_D: {
                goto exit;
            }
            case BACKSPACE_KEY: {
                if (!screen.line_total_x) {
                    break;
                }

                if (screen.lines_y > 0 && screen.x - 1 == 0) {
                    putchar('B');
                    fflush(stdout);
                    break;
                }

                screen.buffer[screen.line_total_x--] = '\0';
                --screen.x;
                ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);

		break;
            }
            case ESCAPE_CHARACTER: {
                break;
            }
            case '\r':
            case '\n': {
		screen_save(&screen);
                continue;
            }
        }

        screen_update(&screen);
    }

exit:
    fflush(stdout);
    screen_exit(&screen);

    return EXIT_SUCCESS;
}
