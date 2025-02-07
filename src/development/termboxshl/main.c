#define TB_IMPL
#include "termbox2.h"
/* always define termbox2 first */

#include <stdlib.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <string.h>

#define LINE_LIMIT 10

struct line {
	bool has_prompt;
	int len;
	char* buffer;
};

struct screen {
    int y;
    int x;
    int max_x;
    int max_y;
    int line_start_y;
    int line_total_x; // buffer_len
    int lines_x[LINE_LIMIT];
    int lines_y;
    struct tb_event ev;
    bool reprint_prompt;
    char* prompt;
    int prompt_len;
    char* buffer;
    char** lines_buffer;
    // struct line* lines;
};

void screen_debug_buffer(struct screen* screen)
{
    int i = 0;
    int written = 0;
    if (screen->line_total_x < screen->max_x) {
        tb_print(screen->x, screen->y + 30, 0, 0, screen->buffer);
    }
    else {
        for (i = 0; i <= screen->lines_y; ++i) {
            tb_write(screen->x, screen->y + 30 + i, 0, 0, screen->buffer + written, screen->lines_x[i] - 1);
            written += screen->lines_x[i] - 1;
        }
    }

    tb_printf(screen->x, screen->y + 31 + i, 0, 0, "(line_total_x, lines_y): (%d, %d)", screen->line_total_x, screen->lines_y);
    tb_printf(screen->x, screen->y + 32 + i, 0, 0, "(max_x, max_y): (%d, %d)", screen->max_x, screen->max_y);

    for (int k = 0; k < screen->lines_y + 1; ++k) {
        tb_printf(screen->x, screen->y + 33 + k + i, 0, 0, "line_x[%d]: %d", k, screen->lines_x[k]);
    }
}

void screen_debug_lines_buffer(struct screen* screen)
{
    for (int i = 0; i < screen->max_y; ++i) {

        tb_print(0, i + 10, TB_GREEN, 0, "> ");
        if (screen->lines_buffer[i][0] == '\0')
            break;
        tb_print(3, i + 10, 0, 0, screen->lines_buffer[i]);
    }

    /*for (int i = 0; i < screen->max_y; ++i) {
	if (screen->lines[i].has_prompt) {
            tb_print(0, i + 10, TB_GREEN, 0, "> ");
	    if (screen->lines[i].buffer[0] == '\0')
                break;
  	    tb_print(3, i + 10, 0, 0, screen->lines[i].buffer);
	}
	else {
	    tb_print(0, i + 10, 0, 0, screen->lines[i].buffer);
	}
    }*/
}

[[nodiscard]]
int screen_init(struct screen* screen)
{
    tb_init();

    screen->max_x = tb_width();
    screen->max_y = tb_height();
    screen->reprint_prompt = false;
    screen->prompt = "> ";
    screen->prompt_len = sizeof("> ") - 1;

    screen->buffer = calloc(PATH_MAX, sizeof(char));
    if (!screen->buffer)
	return EXIT_FAILURE;

    screen->lines_buffer = calloc(screen->max_y, sizeof(char*));
    if (!screen->lines_buffer)
	return EXIT_FAILURE;
    for (int i = 0; i < screen->max_y; ++i) {
        screen->lines_buffer[i] = calloc(PATH_MAX, sizeof(char));
	if (!screen->lines_buffer[i])
	    return EXIT_FAILURE;
    }

    /*screen->lines = malloc(sizeof(struct line));
    for (int i = 0; i < screen->max_y; ++i) {
        screen->lines[i].buffer = calloc(PATH_MAX, sizeof(char));
	if (!screen->lines[i].buffer)
	    return EXIT_FAILURE;
    }*/

    tb_print(screen->x, screen->y, TB_GREEN, 0, "> ");
    // screen->lines[0].has_prompt = true;
    screen->x = screen->prompt_len;
    tb_present();

    return EXIT_SUCCESS;
}

void screen_exit(struct screen* screen)
{
    free(screen->buffer);

    for (int i = 0; i < screen->max_y; ++i) {
        if (screen->lines_buffer[i]) {
            free(screen->lines_buffer[i]);
        }
    }
    free(screen->lines_buffer);

    /*for (int i = 0; i < screen->max_y; ++i) {
        if (screen->lines[i].buffer) {
            free(screen->lines[i].buffer);
        }
    }
    free(screen->lines);*/

    tb_shutdown();
}

void screen_prompt(struct screen* screen)
{
    screen->x = 0;
    tb_print(screen->x, ++screen->y, TB_GREEN, 0, "> ");
    screen->x = screen->prompt_len;
    // screen->lines[screen->y].has_prompt = true;
    // screen_debug_print(screen->buffer, screen);
    screen->line_start_y = screen->y;
    screen->line_total_x = 0;
    screen->lines_y = 0;
    memset(screen->lines_x, 0, sizeof(int) * LINE_LIMIT);
    memset(screen->buffer, 0, sizeof(char) * PATH_MAX);
    screen->reprint_prompt = false;
}

void screen_update(struct screen* screen)
{
    int y = screen->y >= screen->max_y ? screen->max_y - screen->y : screen->y;
    strcpy(screen->lines_buffer[y], screen->buffer);

    if (screen->x >= screen->max_x) {
	screen->lines_x[screen->lines_y] = screen->x;
        ++screen->y; // handle multiline being at end of screen
        ++screen->lines_y;
	screen->x = 0;
    }

    /*if (screen->y >= screen->max_y) {
        memmove(screen->lines_buffer, screen->lines_buffer + 1, screen->max_y);
        for (int i = 0; i < screen->max_y; ++i) {
            tb_print(0, i, TB_GREEN, 0, "> ");
            tb_print(screen->prompt_len, i, 0, 0, screen->lines_buffer[i]);
        }
    }*/
}

int main()
{
    struct screen screen = {0};
    if (screen_init(&screen) != EXIT_SUCCESS)
	return EXIT_FAILURE;

    do {
    	if (screen.reprint_prompt) {
	    screen_prompt(&screen);
    	    tb_present();
    	}
    	tb_poll_event(&screen.ev);

        char new_char = screen.ev.ch == 0 ? ' ' : (char)screen.ev.ch;
        screen.buffer[screen.line_total_x++] = new_char;
	tb_printf(screen.x++, screen.y, 0, 0, "%c", screen.ev.ch);

        switch (screen.ev.key) {
            case 4: {
                screen_debug_lines_buffer(&screen);
                goto exit;
            }
            case 13: {
                screen.lines_x[screen.lines_y] = screen.x;
	        screen.reprint_prompt = true;
                continue;
            }
        }

        screen_update(&screen);
        tb_present();
    } while (1);

exit:
    tb_present();
    tb_poll_event(&screen.ev);
    screen_exit(&screen);

    return 0;
}
