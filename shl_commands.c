/* Handle built-in and running external commands. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shl_output.h"
#include "shl_string.h"
#include "shl_terminal.h"
#include "shl_types.h"

_Bool shl_is_exit_command(struct shl_Args args)
{
	if (shl_string_compare(args.lines[0], "q", args.maxLineSize) == 0)
		return true;
	if (shl_string_compare(args.lines[0], "exit", args.maxLineSize) == 0)
		return true;
	if (shl_string_compare(args.lines[0], "quit", args.maxLineSize) == 0)
		return true;

	return false;
}

uint_fast32_t shl_echo_command(struct shl_Args args)
{
	for (uint_fast32_t i = 1; i < args.count; i++)
		printf("%s ", args.lines[i]);

	if (args.count > 0)
		printf("\n");

	return 1;
}

uint_fast32_t shl_help_command(void)
{
	printf("ncshell by Alex Eski: help\n\n");

	printf("To exit, type q, exit, or quit and press enter.\n");
	printf("You can also use Ctrl+D to exit.\n");

	return 1;
}

uint_fast32_t shl_cd_command(struct shl_Args args)
{
	if (args.lines[1] == NULL)
	{
		chdir(getenv("HOME"));
		return 1;
	}

	if (chdir(args.lines[1]) != 0)
		fprintf(stderr, "shl: could not change directory.\n");

	return 1;
}

uint_fast32_t shl_export_command(struct shl_Args args)
{
	if (args.count < 3)
	{
		fprintf(stderr, "shl: export requires 3 values.\n");
		return 1;
	}

	const char* environment_variable = args.lines[1];
	const char* value = args.lines[2];
	assert(environment_variable != NULL);
	assert(value != NULL);
	// setenv(environment_variable, value, 1);
	return 1;
}

static char** history;
static uint_fast32_t history_position = 0;
static const uint_fast32_t max_history_position = 50;

void shl_history_init(void)
{
	history = malloc(sizeof(char*) * max_history_position);
}

void shl_history_add(char* line, uint_fast32_t length)
{
	if (history_position < max_history_position)
	{
		history[history_position] = malloc(sizeof(char) * length);
		shl_string_copy(history[history_position++], line, length);
	}
}

uint_fast32_t shl_history_command()
{
	for (uint_fast32_t i = 0; i < history_position; i++) {
		printf("%s\n", history[i]);
	}
	return 1;
}

uint_fast32_t shl_launch_process(struct shl_Args args)
{
	#if shl_DEBUG
		shl_debug_launch_process(args);
	#endif /* ifdef shl_DEBUG */

	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		int exec_result = execvp(args.lines[0], args.lines);
		if (exec_result == -1)
		{
			perror(RED "Could not find command" RESET);
			fflush(stdout);
			shl_terminal_init();
			return 0;
		}
		return 1;
	}
	else if (pid < 0)
	{
		perror(RED "Error when forking process" RESET);
		fflush(stdout);
		return 0;
	}
	else
	{
		do {
			waitpid(pid, &status, WUNTRACED);
		}while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

uint_fast32_t shl_execute_command(struct shl_Args args)
{
	if (shl_is_exit_command(args))
		return 0;

	if (shl_string_compare(args.lines[0], "echo", args.maxLineSize) == 0)
		return shl_echo_command(args);

	if (shl_string_compare(args.lines[0], "help", args.maxLineSize) == 0)
		return shl_help_command();

	if (shl_string_compare(args.lines[0], "cd", args.maxLineSize) == 0)
		return shl_cd_command(args);

	if (shl_string_compare(args.lines[0], "export", args.maxLineSize) == 0)
		return shl_export_command(args);

	if (shl_string_compare(args.lines[0], "history", args.maxLineSize) == 0)
		return shl_history_command();

	return shl_launch_process(args);
}


