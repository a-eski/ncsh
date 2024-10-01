#define _POSIX_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "ncsh_terminal.h"
#include "ncsh_args.h"
#include "ncsh_builtin_commands.h"
#include "eskilib/eskilib_string.h"
#include "eskilib/eskilib_colors.h"

#define PIPE_KEY "|"
#define OUTPUT_REDIRECTION_KEY '>'
#define INPUT_REDIRECTION_KEY '<'
#define BACKGROUND_JOB_KEY '&'

uint_fast8_t ncsh_pipe_error(void) {
	perror(RED "Error when piping process" RESET);
	fflush(stdout);
	return 0;
}

uint_fast8_t ncsh_fork_error(void) {
	perror(RED "Error when forking process" RESET);
	fflush(stdout);
	return 0;
}

bool ncsh_any_pipes(struct ncsh_Args args) {
	for (uint_fast8_t i = 0; i < args.count; i++) {
		if (eskilib_string_equals(args.values[i], PIPE_KEY, args.max_line_length))
			return true;
	}

	return false;
}

uint_fast8_t ncsh_execute_piped(struct ncsh_Args args) {
	int file_descriptor_one[2];
	int file_descriptor_two[2];

	uint_fast8_t number_of_commands = 0;
	char *command[256];
	pid_t pid;
	bool end = false;

	uint_fast8_t i = 0; uint_fast8_t j = 0; uint_fast8_t k = 0; uint_fast8_t l = 0;

	while (args.values[l] != NULL){
		if (eskilib_string_equals(args.values[l], PIPE_KEY, args.max_line_length)){
			number_of_commands++;
		}
		l++;
	}
	number_of_commands++;

	while (args.values[j] != NULL && end != true){
		k = 0;

		while (!eskilib_string_equals(args.values[j], PIPE_KEY, args.max_line_length)) {
			command[k] = args.values[j];
			j++;

			if (args.values[j] == NULL) {
				end = true;
				k++;
				break;
			}

			k++;
		}

		command[k] = NULL;
		j++;

		if (i % 2 != 0) {
			if (pipe(file_descriptor_one) != 0)
				return ncsh_pipe_error();
		}
		else {
			if (pipe(file_descriptor_two) != 0)
				return ncsh_pipe_error();
		}

		pid = fork();
		if (pid == -1) {
			if (i != number_of_commands - 1) {
				if (i % 2 != 0)
					close(file_descriptor_one[1]);
				else
					close(file_descriptor_two[1]);
			}

			return ncsh_fork_error();
		}

		if (pid == 0) {
			if (i == 0) { //first command
				dup2(file_descriptor_two[1], STDOUT_FILENO);
			}
			else if (i == number_of_commands - 1) { //last command
				if (number_of_commands % 2 != 0)
					dup2(file_descriptor_one[0], STDIN_FILENO);
				else
					dup2(file_descriptor_two[0], STDIN_FILENO);
			}
			else { //middle command
				if (i % 2 != 0) {
					dup2(file_descriptor_two[0], STDIN_FILENO);
					dup2(file_descriptor_one[1], STDOUT_FILENO);
				}
				else {
					dup2(file_descriptor_one[0], STDIN_FILENO);
					dup2(file_descriptor_two[1], STDOUT_FILENO);
				}
			}

			if (execvp(command[0],command) == -1)
				kill(getpid(), SIGTERM);
		}

		// closing file descriptors
		if (i == 0) {
			close(file_descriptor_two[1]);
		}
		else if (i == number_of_commands - 1) {
			if (number_of_commands % 2 != 0)
				close(file_descriptor_one[0]);
			else
				close(file_descriptor_two[0]);
		}
		else {
			if (i % 2 != 0) {
				close(file_descriptor_two[0]);
				close(file_descriptor_one[1]);
			}
			else {
				close(file_descriptor_one[0]);
				close(file_descriptor_two[1]);
			}
		}

		waitpid(pid,NULL,0);

		i++;
	}

	return 1;
}

uint_fast32_t ncsh_execute_program(char** args) {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror(RED "Could not find command" RESET);
			fflush(stdout);
			ncsh_terminal_init();
			return 0;
		}

		return 1;
	}
	else if (pid < 0) {
		return ncsh_fork_error();
	}
	else {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		return 1;
	}
}

uint_fast32_t ncsh_execute_external(struct ncsh_Args args) {
	if (ncsh_any_pipes(args))
		return ncsh_execute_piped(args);

	return ncsh_execute_program(args.values);
}

uint_fast32_t ncsh_execute(struct ncsh_Args args) {
	if (ncsh_is_exit_command(args))
		return 0;

	if (eskilib_string_equals(args.values[0], "echo", args.max_line_length))
		return ncsh_echo_command(args);

	if (eskilib_string_equals(args.values[0], "help", args.max_line_length))
		return ncsh_help_command();

	if (ncsh_is_cd_command(args))
		return ncsh_cd_command(args);

	if (eskilib_string_equals(args.values[0], "history", args.max_line_length))
		return ncsh_history_command();

	return ncsh_execute_external(args);
}

