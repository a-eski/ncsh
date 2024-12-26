// Copyright (c) ncsh by Alex Eski 2024

#include <stddef.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ncsh_defines.h"
#include "ncsh_builtins.h"

bool ncsh_is_exit_command(struct ncsh_Args* args) {
	if (eskilib_string_equals(args->values[0], "q", args->max_line_length))
		return true;
	else if (eskilib_string_equals(args->values[0], "exit", args->max_line_length))
		return true;
	else if (eskilib_string_equals(args->values[0], "quit", args->max_line_length))
		return true;
	else
		return false;
}

void ncsh_echo_command(struct ncsh_Args* args) {
	for (uint_fast32_t i = 1; i < args->count; ++i)
		printf("%s ", args->values[i]);

	if (args->count > 0)
		putchar('\n');
}

int_fast32_t ncsh_help_command(void) {
	if (write(STDOUT_FILENO, "ncsh by Alex Eski: help\n\n", 25) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	if (write(STDOUT_FILENO, "Builtin Commands {command} {args}\n", 34) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	if (write(STDOUT_FILENO, "q:		To exit, type q, exit, or quit and press enter. You can also use Ctrl+D to exit.\n", 85) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	if (write(STDOUT_FILENO, "cd/z:		You can change directory with cd or z.\n", 46) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	if (write(STDOUT_FILENO, "echo:		You can write things to the screen using echo.\n", 54) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	if (write(STDOUT_FILENO, "history:	You can see your command history using the history command.\n", 69) == -1) {
		perror("ncsh: Error writing to stdout");
		return NCSH_COMMAND_EXIT_FAILURE;
	}

	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

void ncsh_cd_command(struct ncsh_Args* args) {
	if (!args->values[1]) {
		char* home = getenv("HOME");
		if (!home)
			fputs("ncsh: could not change directory.\n", stderr);
		else if (chdir(home) != 0)
			perror("ncsh: could not change directory");

		return;
	}

	if (chdir(args->values[1]) != 0)
		fputs("ncsh: could not change directory.\n", stderr);
}

