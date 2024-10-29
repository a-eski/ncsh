// Copyright (c) ncsh by Alex Eski 2024

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "ncsh_terminal.h"
#include "eskilib/eskilib_colors.h"

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

void ncsh_terminal_size() {}

void ncsh_terminal_move(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

struct ncsh_Coordinates ncsh_terminal_position() {
	return (struct ncsh_Coordinates) { .x = 0, .y = 0 };
}
