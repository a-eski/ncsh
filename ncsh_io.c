/* Copyright Alex Eski 2024 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ncsh_io.h"
#include "eskilib/eskilib_colors.h"

enum ncsh_Hotkey ncsh_get_key(char character) {
	switch (character) {
		case UP_ARROW: { return UP; }
		case DOWN_ARROW: { return DOWN; }
		case RIGHT_ARROW: { return RIGHT; }
		case LEFT_ARROW: { return LEFT; }
		case DELETE_KEY: { return DELETE; }
		default: { return NONE; }
	}
}

void ncsh_write(char* string, uint_fast32_t length) {
	if (write(STDOUT_FILENO, string, length) == -1) {
		perror(RED "Error writing to stdout" RESET);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	fflush(stdout);
}

void ncsh_print_prompt(struct ncsh_Directory prompt_info) {
	char *getcwd_result = getcwd(prompt_info.path, sizeof(prompt_info.path));
	if (getcwd_result == NULL) {
		perror(RED "conch-shell: error when getting current directory" RESET);
		exit(EXIT_FAILURE);
	}

	printf(ncsh_GREEN "%s" WHITE ":" ncsh_CYAN "%s" WHITE "$ ", prompt_info.user, prompt_info.path);
	fflush(stdout);
}

