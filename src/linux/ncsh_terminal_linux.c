#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
// #include <sys/ioctl.h>

#include <termios.h>
#include <unistd.h>

#include "../eskilib/eskilib_colors.h"
#include "../ncsh_terminal_base.h"

static struct termios terminal;
static struct termios original_terminal;

void ncsh_terminal_linux_reset(void) {
	fflush(stdout);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal) != 0) {
		perror(RED "ncsh: Could not restore terminal settings" RESET);
	}
}

void ncsh_terminal_linux_init(void) {
	// if (uv_guess_handle(fileno(stream)) != UV_TTY)
	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Not running in a terminal.\n");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(STDIN_FILENO, &original_terminal) != 0) {
		perror(RED "ncsh: Could not get terminal settings" RESET);
		exit(EXIT_FAILURE);
	}

	terminal = original_terminal;
	terminal.c_lflag &= ~(ICANON|ECHO);
	terminal.c_cc[VMIN] = 1;
	terminal.c_cc[VTIME] = 0;
	// terminal.c_cc[VEOF] = CTRL_D;
	// terminal.c_cc[VINTR] = CTRL_C;
	// terminal.c_cc[VKILL] = CTRL_U;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) != 0) {
		perror(RED "ncsh: Could not set terminal settings" RESET);
	}

	signal(SIGHUP, SIG_DFL); // Stops the process if the terminal is closed
}

struct ncsh_Coordinates ncsh_terminal_linux_size(void) {
	// todo: look at getenv("LINES"), getenv("COLUMNS") since ioctl is not portable
	/*struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	return (struct ncsh_Coordinates) { .x = w.ws_col, .y = w.ws_row };*/
	return (struct ncsh_Coordinates) { .x = 0, .y = 0 };
}
