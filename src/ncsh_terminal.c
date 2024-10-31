// Copyright (c) ncsh by Alex Eski 2024

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "ncsh_terminal.h"
#include "ncsh_io.h"
#include "eskilib/eskilib_colors.h"

#define TERMINAL_RETURN 'R'

static struct termios terminal;
static struct termios original_terminal;

void ncsh_terminal_reset(void) {
	fflush(stdout);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal) != 0) {
		perror(RED "Could not restore terminal settings" RESET);
	}
}

void ncsh_terminal_init(void) {
	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Not running in a terminal.\n");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(STDIN_FILENO, &original_terminal) != 0) {
		perror(RED "Could not get terminal settings" RESET);
		exit(EXIT_FAILURE);
	}
	atexit(ncsh_terminal_reset);

	terminal = original_terminal;
	terminal.c_lflag &= ~(ICANON|ECHO);
	terminal.c_cc[VMIN] = 1;
	terminal.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) != 0) {
		perror(RED "Could not set terminal settings" RESET);
	}
}

struct ncsh_Coordinates ncsh_terminal_size() {
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return (struct ncsh_Coordinates) { .x = w.ws_col, .y = w.ws_row };
}

void ncsh_terminal_move(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

struct ncsh_Coordinates ncsh_terminal_position() {
	constexpr int buffer_length = 30;
	char buffer[buffer_length] = {};
	int_fast32_t i;
	int_fast32_t power = 0;
	char character;
	struct ncsh_Coordinates cursor_position = {};

	ncsh_write(GET_CURSOR_POSITION, GET_CURSOR_POSITION_LENGTH);

	for (i = 0; i < buffer_length && character != TERMINAL_RETURN; ++i) {
		if (read(STDIN_FILENO, &character, 1) == -1)
		{
			perror("ncsh: Could not get cursor position.");
			return cursor_position;
		}
		buffer[i] = character;
	}

	if (i < 2 || i == buffer_length - 1)
	{
		return cursor_position;
	}

	for (i -= 2, power = 1; buffer[i] != ';'; i--, power *= 10)
		cursor_position.x += (buffer[i] - '0') * power;

	for (i--, power = 1; buffer[i] != '['; i--, power *= 10)
		cursor_position.y += (buffer[i] - '0') * power;

	return cursor_position;
}
