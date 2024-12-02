// Copyright (c) ncsh by Alex Eski 2024

#define _POSIX_SOURCE
#include <stdint.h>
#include <linux/limits.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "ncsh_terminal.h"
#include "ncsh_args.h"
#include "ncsh_builtins.h"
#include "ncsh_vm.h"
#include "eskilib/eskilib_string.h"
#include "eskilib/eskilib_colors.h"

#define PIPE_KEY "|"
#define OUTPUT_REDIRECTION_KEY '>'
#define INPUT_REDIRECTION_KEY '<'
#define BACKGROUND_JOB_KEY '&'

/*struct ncsh_VM {
	enum ncsh_Ops op;
	int original_stdout;
	int original_stdin;
	int original_stderr;
	int fd_one[2];
	int fd_two[2];
	char* file;
};*/

struct ncsh_Output_Redirect_IO {
	int fd;
	int original_stdout;
	int original_stderr;
};

struct ncsh_Pipe_IO {
	int fd_one[2];
	int fd_two[2];
};

struct ncsh_Output_Redirect_IO ncsh_output_redirection_start(char* file) {
	assert(file != NULL);

	int file_descriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	struct ncsh_Output_Redirect_IO io;

	io.original_stdout = dup(STDOUT_FILENO);
	io.original_stderr = dup(STDERR_FILENO);
	dup2(file_descriptor, STDOUT_FILENO);
	dup2(file_descriptor, STDERR_FILENO);

	close(file_descriptor);

	return io;
}

void ncsh_output_redirection_stop(struct ncsh_Output_Redirect_IO io) {
	dup2(io.original_stdout, STDOUT_FILENO);
	dup2(io.original_stderr, STDERR_FILENO);
}

int_fast32_t ncsh_pipe_start(struct ncsh_Pipe_IO* pipes, uint_fast32_t command_position) {
	assert(pipes != NULL);

	if (command_position % 2 != 0) {
		if (pipe(pipes->fd_one) != 0) {
			perror(RED "ncsh: Error when piping process" RESET);
			fflush(stdout);
			return -1;
		}
	}
	else {
		if (pipe(pipes->fd_two) != 0) {
			perror(RED "ncsh: Error when piping process" RESET);
			fflush(stdout);
			return -1;
		}
	}

	return 1;
}

int_fast32_t ncsh_pipe_fork_failure(struct ncsh_Pipe_IO* pipes, uint_fast32_t command_position, uint_fast32_t number_of_commands) {
	assert(pipes != NULL);

	if (command_position != number_of_commands - 1) {
		if (command_position % 2 != 0)
			close(pipes->fd_one[1]);
		else
			close(pipes->fd_two[1]);
	}

	perror(RED "ncsh: Error when forking process" RESET);
	fflush(stdout);
	return -1;
}

void ncsh_pipe_connect(struct ncsh_Pipe_IO* pipes, uint_fast32_t command_position, uint_fast32_t number_of_commands) {
	assert(pipes != NULL);

	if (command_position == 0) { //first command
		dup2(pipes->fd_two[1], STDOUT_FILENO);
	}
	else if (command_position == number_of_commands - 1) { //last command
		if (number_of_commands % 2 != 0)
			dup2(pipes->fd_one[0], STDIN_FILENO);
		else
			dup2(pipes->fd_two[0], STDIN_FILENO);
	}
	else { //middle command
		if (command_position % 2 != 0) {
			dup2(pipes->fd_two[0], STDIN_FILENO);
			dup2(pipes->fd_one[1], STDOUT_FILENO);
		}
		else {
			dup2(pipes->fd_one[0], STDIN_FILENO);
			dup2(pipes->fd_two[1], STDOUT_FILENO);
		}
	}
}

void ncsh_pipe_stop(struct ncsh_Pipe_IO* pipes, uint_fast32_t command_position, uint_fast32_t number_of_commands) {
	assert(pipes != NULL);

	if (command_position == 0) {
		close(pipes->fd_two[1]);
	}
	else if (command_position == number_of_commands - 1) {
		if (number_of_commands % 2 != 0) {
			close(pipes->fd_one[0]);
		}
		else {
			close(pipes->fd_two[0]);
		}
	}
	else {
		if (command_position % 2 != 0) {
			close(pipes->fd_two[0]);
			close(pipes->fd_one[1]);
		}
		else {
			close(pipes->fd_one[0]);
			close(pipes->fd_two[1]);
		}
	}
}

int_fast32_t ncsh_vm(struct ncsh_Args args) {
	assert(args.values != NULL);
	assert(args.ops != NULL);
	assert(args.count != 0);
	assert(args.max_line_length != 0);

	struct ncsh_Pipe_IO pipes_io = {0};
	struct ncsh_Output_Redirect_IO output_io = {0};

	char* buffer[MAX_INPUT] = {0};
	pid_t pid = 0;
	bool end = false;
	enum ncsh_Ops op_current = OP_NONE;

	uint_fast32_t number_of_commands = 0;
	uint_fast32_t command_position = 0;
	uint_fast32_t args_position = 0;
	uint_fast32_t buffer_position = 0;

	char* file = NULL;
	uint_fast32_t output_redirect_found = 0;
	for (uint_fast32_t l = 0; l < args.count; ++l) {
		if (args.ops[l] == OP_OUTPUT_REDIRECTION) {
			file = args.values[l + 1];
			output_redirect_found = l;
		}
		else if (args.ops[l] == OP_PIPE) {
			++number_of_commands;
		}
	}
	++number_of_commands;

	if (file != NULL) {
		free(args.values[output_redirect_found]);
		args.values[output_redirect_found] = NULL;
		output_io = ncsh_output_redirection_start(file);
	}

	while (args.values[args_position] != NULL && end != true) {
		buffer_position = 0;

		while (args.ops[args_position] == OP_CONSTANT) {
			buffer[buffer_position] = args.values[args_position];
			++args_position;

			if (args.values[args_position] == NULL) {
				end = true;
				++buffer_position;
				break;
			}

			++buffer_position;
		}

		if (!end) {
			op_current = args.ops[args_position];
		}

		buffer[buffer_position] = NULL;
		if (buffer[0] == NULL) {
			return 1;
		}

		++args_position;

		if (op_current == OP_PIPE && !end) {
			if (!ncsh_pipe_start(&pipes_io, command_position))
				return -1;
		}

		pid = fork();
		if (pid == -1)
			return ncsh_pipe_fork_failure(&pipes_io, command_position, number_of_commands);

		if (pid == 0) {
			if (op_current == OP_PIPE)
				ncsh_pipe_connect(&pipes_io, command_position, number_of_commands);


			if (execvp(buffer[0], buffer) == -1) {
				end = true;
				perror(RED "ncsh: Could not find command or directory" RESET);
				fflush(stdout);
				kill(getpid(), SIGTERM);
			}
		}

		if (op_current == OP_PIPE)
			ncsh_pipe_stop(&pipes_io, command_position, number_of_commands);

		int status;
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		++command_position;
	}

	if (file != NULL)
		ncsh_output_redirection_stop(output_io);

	return 1;
}

int_fast32_t ncsh_vm_execute(struct ncsh_Args args, struct ncsh_History* history) {
	assert(args.values != NULL);
	assert(args.ops != NULL);
	assert(history != NULL);

	if (ncsh_is_exit_command(args))
		return 0;

	if (eskilib_string_equals(args.values[0], "echo", args.max_line_length))
		return ncsh_echo_command(args);

	if (eskilib_string_equals(args.values[0], "help", args.max_line_length))
		return ncsh_help_command();

	if (ncsh_is_cd_command(args))
		return ncsh_cd_command(args);

	if (eskilib_string_equals(args.values[0], "history", args.max_line_length))
		return ncsh_history_command(history);

	ncsh_terminal_reset(); //reset terminal settings since a lot of terminal programs use canonical mode

	int_fast32_t result = ncsh_vm(args);

	ncsh_terminal_init(); //back to noncanonical mode

	return result;
}

