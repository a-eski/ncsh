#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "ncsh_terminal.h"
#include "ncsh_args.h"
#include "eskilib/eskilib_string.h"
#include "eskilib/eskilib_colors.h"

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

bool ncsh_args_any_shell_commands(struct ncsh_Args args) {
	for (uint_fast32_t i = 0; i < args.count; i++) {
		if (args.op_codes[i] != OP_NONE)
			return true;
	}

	return false;
}

// uint_fast32_t ncsh_execute_piped(struct ncsh_Args args) {
// 	pid_t pid_one;
// 	pid_t pid_two;
// 	int pipe_destination[2];
//
// 	// char** args_one;
// 	// args_one = malloc(sizeof(char*) * args.count);
// 	// uint_fast32_t args_one_index = 0;
// 	// for (uint_fast32_t args_one_index = 0;
// 	// 	args_one_index < args.count && args.lines[args_one_index] != NULL;
// 	// 	args_one_index++) {
// 	// 	if (args.shellCommands[args_one_index] == NO_COMMAND) {
// 	// 		args_one[args_one_index] = malloc(sizeof(char) * (args_one_index + 1));
// 	// 		eskilib_string_copy(args_one[args_one_index], args.lines[args_one_index], args.maxLineSize);
// 	// 	}
// 	// 	else
// 	// 		break;
// 	// }
// 	// printf("args_one[0]: %s", args_one[0]);
// 	//
// 	// char** args_two;
// 	// uint_fast32_t position = 0;
// 	// args_two = malloc(sizeof(char*) * (args.count - args_one_index));
// 	// for (uint_fast32_t args_two_index = args_one_index;
// 	// 	args_two_index < args.count && args.lines[args_two_index] != NULL;
// 	// 	args_two_index++) {
// 	// 	if (args.shellCommands[args_two_index] == NO_COMMAND) {
// 	// 		args_two[position++] = malloc(sizeof(char) * (args_one_index + 1));
// 	// 		eskilib_string_copy(args_two[args_two_index], args.lines[args_two_index], args.maxLineSize);
// 	// 	}
// 	// 	else
// 	// 		break;
// 	// }
// 	// printf("args_two[0]: %s", args_two[0]);
// 	//
// 	if (pipe(pipe_destination) < 0) {
// 		printf(RED "Could not initialize pipe.\n" RESET);
// 		fflush(stdout);
// 		return 0;
// 	}
// 	pid_one = fork();
// 	if (pid_one == 0) {
// 		close(pipe_destination[0]);
// 		dup2(pipe_destination[1], STDOUT_FILENO);
// 		close(pipe_destination[1]);
//
// 		if (execvp(args.values[0], &args.values[0].value) < 0) {
// 			printf(RED "Error when executing first command" RESET);
// 			return 1;
// 		}
// 	}
// 	else if (pid_one < 0) {
// 		perror(RED "Error when forking process" RESET);
// 		fflush(stdout);
// 		return 0;
// 	}
// 	else {
// 		pid_two = fork();
// 		if (pid_two == 0) {
// 			close(pipe_destination[1]);
// 			dup2(pipe_destination[0], STDIN_FILENO);
// 			close(pipe_destination[0]);
// 			
// 			if (execvp(args.values[0], &args.values[0].value) < 0) {
// 				printf(RED "Error when executing second command" RESET);
// 				return 1;
// 			}
// 		}
// 		else if (pid_two < 0) {
// 			perror(RED "Error when forking process" RESET);
// 			fflush(stdout);
// 			return 0;
// 		}
// 		else {
// 			wait(NULL);
// 			wait(NULL);
// 		}
// 	}
//
// 	return 1;
// }

uint_fast32_t ncsh_execute(char** args) {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		int exec_result = execvp(args[0], args);
		if (exec_result == -1)
		{
			perror(RED "Could not find command" RESET);
			fflush(stdout);
			ncsh_terminal_init();
			return 0;
		}
		return 1;
	} else if (pid < 0) {
		perror(RED "Error when forking process" RESET);
		fflush(stdout);
		return 0;
	} else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		return 1;
	}
}

uint_fast32_t ncsh_execute_command(struct ncsh_Args args) {
	if (ncsh_is_exit_command(args))
		return 0;

	if (eskilib_string_equals(args.values[0], "echo", args.max_line_length))
		return ncsh_echo_command(args);

	if (eskilib_string_equals(args.values[0], "help", args.max_line_length))
		return ncsh_help_command();

	if (eskilib_string_equals(args.values[0], "cd", args.max_line_length))
		return ncsh_cd_command(args);

	if (eskilib_string_equals(args.values[0], "history", args.max_line_length))
		return ncsh_history_command();

	// if (args.op_code_found)
	// 	return ncsh_execute_piped(args);

	return ncsh_execute(args.values);
}

