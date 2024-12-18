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
#include "ncsh_vm.h"
#include "ncsh_defines.h"
#include "eskilib/eskilib_colors.h"

struct ncsh_Output_Redirect_IO {
	int fd;
	int original_stdout;
	int original_stderr;
};

struct ncsh_Input_Redirect_IO {
	int fd;
	int original_stdin;
};

struct ncsh_Pipe_IO {
	int fd_one[2];
	int fd_two[2];
};

struct ncsh_Vm {
	struct ncsh_Output_Redirect_IO output_redirect_io;
	struct ncsh_Input_Redirect_IO input_redirect_io;
	struct ncsh_Pipe_IO pipes_io;
};

void ncsh_output_redirection_start(char* file, struct ncsh_Output_Redirect_IO* io) {
	assert(file != NULL);

	int file_descriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file_descriptor == -1) {
		io->fd = -1;
	}

	io->original_stdout = dup(STDOUT_FILENO);
	io->original_stderr = dup(STDERR_FILENO);
	dup2(file_descriptor, STDOUT_FILENO);
	dup2(file_descriptor, STDERR_FILENO);

	close(file_descriptor);
}

void ncsh_output_redirection_stop(struct ncsh_Output_Redirect_IO* io) {
	dup2(io->original_stdout, STDOUT_FILENO);
	dup2(io->original_stderr, STDERR_FILENO);
}

void ncsh_input_redirection_start(char* file, struct ncsh_Input_Redirect_IO* io) {
	assert(file != NULL);

	int file_descriptor = open(file, O_RDONLY);
	if (file_descriptor == -1) {
		io->fd = -1;
	}

	io->original_stdin = dup(STDIN_FILENO);
	dup2(file_descriptor, STDIN_FILENO);

	close(file_descriptor);
}

void ncsh_input_redirection_stop(int original_stdin) {
	dup2(original_stdin, STDIN_FILENO);
}

int_fast32_t ncsh_pipe_start(uint_fast32_t command_position, struct ncsh_Pipe_IO* pipes) {
	assert(pipes != NULL);

	if (command_position % 2 != 0) {
		if (pipe(pipes->fd_one) != 0) {
			perror(RED "ncsh: Error when piping process" RESET);
			fflush(stdout);
			return NCSH_COMMAND_EXIT_FAILURE;
		}
	}
	else {
		if (pipe(pipes->fd_two) != 0) {
			perror(RED "ncsh: Error when piping process" RESET);
			fflush(stdout);
			return NCSH_COMMAND_EXIT_FAILURE;
		}
	}

	return NCSH_COMMAND_CONTINUE;
}

int_fast32_t ncsh_pipe_fork_failure(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes) {
	assert(pipes != NULL);

	if (command_position != number_of_commands - 1) {
		if (command_position % 2 != 0)
			close(pipes->fd_one[1]);
		else
			close(pipes->fd_two[1]);
	}

	perror(RED "ncsh: Error when forking process" RESET);
	fflush(stdout);
	return NCSH_COMMAND_EXIT_FAILURE;
}

void ncsh_pipe_connect(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes) {
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

void ncsh_pipe_stop(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes) {
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

int_fast32_t ncsh_syntax_error(const char* message, size_t message_length) {
	if (write(STDIN_FILENO, message, message_length) == -1)
		return NCSH_COMMAND_EXIT_FAILURE;

	return NCSH_COMMAND_CONTINUE;
}

struct ncsh_Tokens {
	uint_fast32_t output_redirect_found;
	uint_fast32_t input_redirect_found;
	uint_fast32_t number_of_pipe_commands;
	char* output_file;
	char* input_file;
};

int_fast32_t ncsh_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens) {
	if (args->ops[0] == OP_PIPE)
		return ncsh_syntax_error("ncsh: Invalid syntax: found pipe ('|') as first argument. Correct usage of pipes is 'ls | sort'.\n", 97);
	else if (args->ops[args->count - 1] == OP_PIPE)
		return ncsh_syntax_error("ncsh: Invalid syntax: found pipe ('|') as last argument. Correct usage of pipes is 'ls | sort'.\n", 96);

	for (uint_fast32_t l = 0; l < args->count; ++l) {
		if (args->ops[l] == OP_OUTPUT_REDIRECTION) {
			if (l + 1 >= args->count) {
				return ncsh_syntax_error("ncsh: Invalid syntax: found no filename after output redirect symbol '>'.\n", 74);
			}

			tokens->output_file = args->values[l + 1];
			tokens->output_redirect_found = l;
		}
		else if (args->ops[l] == OP_INPUT_REDIRECTION) {
			if (l + 1 >= args->count) {
				return ncsh_syntax_error("ncsh: Invalid syntax: found no filename before input redirect symbol '<'.\n", 74);
			}

			tokens->input_file = args->values[l + 1];
			tokens->input_redirect_found = l;
		}
		else if (args->ops[l] == OP_PIPE) {
			++tokens->number_of_pipe_commands;
		}
	}
	++tokens->number_of_pipe_commands;

	if (tokens->output_redirect_found && tokens->input_redirect_found) {
		return ncsh_syntax_error("ncsh: Invalid syntax: found both input and output redirects symbols ('<' and '>', respectively).\n", 97);
	}

	return NCSH_COMMAND_CONTINUE;
}

int_fast32_t ncsh_vm(struct ncsh_Args* args) {
	struct ncsh_Vm vm = {0};
	char* buffer[MAX_INPUT] = {0};
	pid_t pid = 0;
	bool end = false;
	enum ncsh_Ops op_current = OP_NONE;

	struct ncsh_Tokens tokens = {0};
	if (ncsh_tokenize(args, &tokens) != NCSH_COMMAND_CONTINUE)
		return NCSH_COMMAND_EXIT_FAILURE;

	uint_fast32_t command_position = 0;
	uint_fast32_t args_position = 0;
	uint_fast32_t buffer_position = 0;

	if (tokens.output_file && tokens.output_redirect_found) {
		free(args->values[tokens.output_redirect_found]);
		args->values[tokens.output_redirect_found] = NULL;
		ncsh_output_redirection_start(tokens.output_file, &vm.output_redirect_io);
		if (vm.output_redirect_io.fd == -1) {
			printf("ncsh: Invalid file handle '%s': could not open file for output redirection, do you have permission to open the file?\n",
	  			tokens.output_file);
			return NCSH_COMMAND_CONTINUE;
		}
	}
	else if (tokens.input_file && tokens.input_redirect_found) {
		free(args->values[tokens.input_redirect_found]);
		args->values[tokens.input_redirect_found] = NULL;
		ncsh_input_redirection_start(tokens.input_file, &vm.input_redirect_io);
		if (vm.input_redirect_io.fd == -1) {
			printf("ncsh: Invalid file handle '%s': could not open file for input redirection, does the file exist?\n",
				tokens.input_file);
			return NCSH_COMMAND_CONTINUE;
		}
	}

	while (args->values[args_position] != NULL && end != true) {
		buffer_position = 0;

		while (args->ops[args_position] == OP_CONSTANT) {
			buffer[buffer_position] = args->values[args_position];
			++args_position;

			if (args->values[args_position] == NULL) {
				end = true;
				++buffer_position;
				break;
			}

			++buffer_position;
		}

		if (!end) {
			op_current = args->ops[args_position];
		}

		buffer[buffer_position] = NULL;
		if (buffer[0] == NULL) {
			return NCSH_COMMAND_CONTINUE;
		}

		++args_position;

		if (op_current == OP_PIPE && !end) {
			if (!ncsh_pipe_start(command_position, &vm.pipes_io))
				return NCSH_COMMAND_EXIT_FAILURE;
		}

		pid = fork();
		if (pid == -1)
			return ncsh_pipe_fork_failure(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);

		if (pid == 0) {
			if (op_current == OP_PIPE)
				ncsh_pipe_connect(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);


			if (execvp(buffer[0], buffer) == -1) {
				end = true;
				perror(RED "ncsh: Could not find command or directory" RESET);
				fflush(stdout);
				kill(getpid(), SIGTERM);
			}
		}

		if (op_current == OP_PIPE)
			ncsh_pipe_stop(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);

		int status;
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		++command_position;
	}

	if (tokens.output_file && tokens.output_redirect_found)
		ncsh_output_redirection_stop(&vm.output_redirect_io);
	else if (tokens.input_file && tokens.input_redirect_found)
		ncsh_input_redirection_stop(vm.input_redirect_io.original_stdin);

	return NCSH_COMMAND_CONTINUE;
}

int_fast32_t ncsh_vm_execute(struct ncsh_Args* args) {
	assert(args->values != NULL);
	assert(args->ops != NULL);
	assert(args->values[0] != NULL);
	assert(args->count != 0);
	assert(args->max_line_length != 0);

	ncsh_terminal_reset(); //reset terminal settings since a lot of terminal programs use canonical mode

	int_fast32_t result = ncsh_vm(args);

	ncsh_terminal_init(); //back to noncanonical mode

	return result;
}

