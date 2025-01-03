// Copyright (c) ncsh by Alex Eski 2024

#include <stdio.h>
#include <unistd.h>

#include "ncsh_terminal.h"
#include "eskilib/eskilib_colors.h"

#if defined(linux) || defined(__unix__)
#include "linux/ncsh_terminal_linux.c"
#else
#include "ncsh_terminal_msys2.c"
#endif

#define TERMINAL_RETURN 'R'
#define T_BUFFER_LENGTH 30

void ncsh_terminal_reset(void) {
	#if defined(linux) || defined(__unix__)
		ncsh_terminal_linux_reset();
	#endif
}

void ncsh_terminal_init(void) {
	#if defined(linux) || defined(__unix__)
		ncsh_terminal_linux_init();
	#endif
}

struct ncsh_Coordinates ncsh_terminal_size(void) {
	#if defined(linux) || defined(__unix__)
		return ncsh_terminal_linux_size();
	#else
		return (struct ncsh_Coordinates){ .x = 0, .y = 0 };
	#endif
}

void ncsh_terminal_move(int x, int y) {
	printf("\033[%d;%dH", y, x);
}

void ncsh_terminal_move_right(uint_fast32_t i) {
	printf("\033[%luC", i);
}

void ncsh_terminal_move_left(uint_fast32_t i) {
	printf("\033[%luD", i);
}

void ncsh_terminal_move_up(uint_fast32_t i) {
	printf("\033[%luA", i);
}

void ncsh_terminal_move_down(uint_fast32_t i) {
	printf("\033[%luB", i);
}

struct ncsh_Coordinates ncsh_terminal_position(void) {
	char buffer[T_BUFFER_LENGTH] = {0};
	int_fast32_t i = 0;
	int_fast32_t power = 0;
	char character = 0;
	struct ncsh_Coordinates cursor_position = {0};

	if (write(STDOUT_FILENO, GET_CURSOR_POSITION, GET_CURSOR_POSITION_LENGTH) == -1) {
		perror(RED "ncsh: Error writing to stdout" RESET);
		fflush(stdout);
		return cursor_position;
	}


	for (i = 0; i < T_BUFFER_LENGTH && character != TERMINAL_RETURN; ++i) {
		if (read(STDIN_FILENO, &character, 1) == -1)
		{
			perror(RED "ncsh: Could not get cursor position" RESET);
			return cursor_position;
		}
		buffer[i] = character;
	}

	if (i < 2 || i == T_BUFFER_LENGTH - 1)
	{
		return cursor_position;
	}

	for (i -= 2, power = 1; buffer[i] != ';'; i--, power *= 10)
		cursor_position.x += (buffer[i] - '0') * power;

	for (i--, power = 1; buffer[i] != '['; i--, power *= 10)
		cursor_position.y += (buffer[i] - '0') * power;

	return cursor_position;
}

