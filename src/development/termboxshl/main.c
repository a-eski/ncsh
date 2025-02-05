#include <linux/limits.h>
#include <stdbool.h>

#define TB_IMPL
#include "termbox2.h"

#define LINE_LIMIT 10

struct screen {
    int y;
    int x;
    int max_x;
    int max_y;
    int line_start_y;
    int line_total_x;
    int lines_x[LINE_LIMIT];
    int lines_y;
    bool reprint_prompt;
    char* prompt;
    int prompt_len;
    char* buffer;
    struct tb_event ev;
};

void screen_init(struct screen* screen)
{
    tb_init();

    screen->buffer = calloc(PATH_MAX, sizeof(char));

    screen->max_x = tb_width();
    screen->max_y = tb_height();
    screen->reprint_prompt = false;
    screen->prompt = "> ";
    screen->prompt_len = sizeof("> ") - 1;
}

void screen_exit(struct screen* screen)
{
    free(screen->buffer);
    tb_shutdown();
}

void print(char* buffer, struct screen* screen)
{
    int i = 0;
    int written = 0;
    if (screen->line_total_x < screen->max_x) {
        tb_print(screen->x, screen->y + 30, 0, 0, buffer);
    }
    else {
        for (i = 0; i <= screen->lines_y; ++i) {
            tb_write(screen->x, screen->y + 30 + i, 0, 0, buffer + written, screen->lines_x[i] - 1);
            written += screen->lines_x[i] - 1;
        }
    }

    tb_printf(screen->x, screen->y + 31 + i, 0, 0, "(line_total_x, lines_y): (%d, %d)", screen->line_total_x, screen->lines_y);
    tb_printf(screen->x, screen->y + 32 + i, 0, 0, "(max_x, max_y): (%d, %d)", screen->max_x, screen->max_y);

    for (int k = 0; k < screen->lines_y + 1; ++k) {
        tb_printf(screen->x, screen->y + 33 + k + i, 0, 0, "line_x[%d]: %d", k, screen->lines_x[k]);
    }
}

int main() {
    struct screen screen = {0};
    screen_init(&screen);

    tb_print(screen.x, screen.y, TB_GREEN, 0, screen.prompt);
    screen.x = screen.prompt_len;
    do {
    	if (screen.reprint_prompt) {
		screen.x = 0;
    		tb_print(screen.x, ++screen.y, TB_GREEN, 0, "> ");
    		screen.x = screen.prompt_len;
		print(screen.buffer, &screen);
                screen.line_start_y = screen.y;
		screen.line_total_x = 0;
                screen.lines_y = 0;
	        memset(screen.lines_x, 0, sizeof(int) * LINE_LIMIT);
                memset(screen.buffer, 0, sizeof(char) * PATH_MAX);
		screen.reprint_prompt = false;
    	}

    	tb_present();
    	tb_poll_event(&screen.ev);

        screen.buffer[screen.line_total_x++] = (char)screen.ev.ch;
	tb_printf(screen.x++, screen.y, 0, 0, "%c", screen.ev.ch);

        switch (screen.ev.key) {
            case 4: {
                goto exit;
            }
            case 13: {
                screen.lines_x[screen.lines_y] = screen.x;
	        screen.reprint_prompt = true;
                continue;
            }
            /*case 8: {
                --screen.line_total_x;
                --screen.x;
                memmove(&screen.buffer[screen.x], &screen.buffer[screen.x + 1], screen.line_total_x - screen.x);
                tb_print(screen.x, screen.y, 0, 0, &screen.buffer[screen.x]);
            }*/
        }

        if (screen.x >= screen.max_x) {
	    screen.lines_x[screen.lines_y] = screen.x;
            ++screen.y;
            ++screen.lines_y;
	    screen.x = 0;
        }

        tb_present();
    } while (1);

exit:
    screen_exit(&screen);

    return 0;
}
