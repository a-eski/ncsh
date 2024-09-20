#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shl_terminal.h"
#include "shl_output.h"

static struct termios terminal;
static struct termios original_terminal;

void shl_terminal_reset(void)
{
	fflush(stdout);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal) != 0)
	{
		perror(RED "Could not restore terminal settings" RESET);
	}
}

void shl_terminal_init(void)
{
	if (!isatty(STDIN_FILENO))
	{
		fprintf(stderr, "Not running in a terminal.\n");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(STDIN_FILENO, &original_terminal) != 0)
	{
		perror(RED "Could not get terminal settings" RESET);
		exit(EXIT_FAILURE);
	}
	atexit(shl_terminal_reset);

	terminal = original_terminal;
	terminal.c_lflag &= ~(ICANON|ECHO);
	terminal.c_cc[VMIN] = 1;
	terminal.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) != 0)
	{
		perror(RED "Could not set terminal settings" RESET);
	}
}

