#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"

bool ncsh_is_exit_command(struct ncsh_Args args) {
	if (eskilib_string_equals(args.values[0], "q", args.max_line_length))
		return true;
	else if (eskilib_string_equals(args.values[0], "exit", args.max_line_length))
		return true;
	else if (eskilib_string_equals(args.values[0], "quit", args.max_line_length))
		return true;
	else
		return false;
}

uint_fast32_t ncsh_echo_command(struct ncsh_Args args) {
	for (uint_fast32_t i = 1; i < args.count; i++)
		printf("%s ", args.values[i]);

	if (args.count > 0)
		printf("\n");

	return 1;
}

uint_fast32_t ncsh_help_command(void) {
	printf("ncshell by Alex Eski: help\n\n");

	printf("To exit, type q, exit, or quit and press enter.\n");
	printf("You can also use Ctrl+D to exit.\n");

	return 1;
}

uint_fast32_t ncsh_cd_command(struct ncsh_Args args) {
	if (args.values[1][0] == '\0') {
		char* home = getenv("HOME");
		if (home == NULL || chdir(home) != 0)
			fprintf(stderr, "ncsh: could not change directory.\n");

		return 1;
	}

	if (chdir(args.values[1]) != 0)
		fprintf(stderr, "ncsh: could not change directory.\n");

	return 1;
}

static char** history;
static uint_fast32_t history_position = 0;
static const uint_fast32_t max_history_position = 50;

void ncsh_history_init(void) {
	history = malloc(sizeof(char*) * max_history_position);
}

void ncsh_history_add(char* line, uint_fast32_t length) {
	if (history_position < max_history_position) {
		history[history_position] = malloc(sizeof(char) * length);
		eskilib_string_copy(history[history_position++], line, length);
	}
}

uint_fast32_t ncsh_history_command(void) {
	for (uint_fast32_t i = 0; i < history_position; i++) {
		printf("%s\n", history[i]);
	}
	return 1;
}

