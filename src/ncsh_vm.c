// Copyright (c) ncsh by Alex Eski 2024

#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <linux/limits.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <bits/types/siginfo_t.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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

struct ncsh_Tokens {
	uint_fast32_t output_redirect_found_index;
	uint_fast32_t input_redirect_found_index;
	uint_fast32_t number_of_pipe_commands;
	char* output_file;
	char* input_file;
};

static pid_t ncsh_internal_child_pid = 0;

static inline void  ncsh_set_child_pid(pid_t p) {
	__atomic_store_n(&ncsh_internal_child_pid, p, __ATOMIC_SEQ_CST);
}

static inline pid_t ncsh_get_child_pid(void) {
	return __atomic_load_n(&ncsh_internal_child_pid, __ATOMIC_SEQ_CST);
}

static void ncsh_vm_signal_handler(int signum, siginfo_t* info, void* context) {
	(void)context;
	const pid_t target = ncsh_get_child_pid();

	if (target != 0 && info->si_pid != target) {
		if (kill(target, signum) == 0) {
			if (write(STDOUT_FILENO, "\n", 1) == -1) //write is async safe, do not use fflush, putchar, prinft
				perror("ncsh: Error writing to standard output while processing a signal");
		}
	}
}

static int ncsh_vm_signal_forward(const int signum) {
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = ncsh_vm_signal_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(signum, &act, NULL))
        return errno;

    return 0;
}

void ncsh_stdout_redirection_start(char* file, struct ncsh_Output_Redirect_IO* io) {
	assert(file != NULL);

	int file_descriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file_descriptor == -1) {
		io->fd = -1;
	}

	io->original_stdout = dup(STDOUT_FILENO);
	dup2(file_descriptor, STDOUT_FILENO);

	close(file_descriptor);
}

void ncsh_stdout_redirection_stop(struct ncsh_Output_Redirect_IO* io) {
	dup2(io->original_stdout, STDOUT_FILENO);
}

void ncsh_stderr_redirection_start(char* file, struct ncsh_Output_Redirect_IO* io) {
	assert(file != NULL);

	int file_descriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (file_descriptor == -1) {
		io->fd = -1;
	}

	io->original_stderr = dup(STDERR_FILENO);
	dup2(file_descriptor, STDERR_FILENO);

	close(file_descriptor);
}

void ncsh_stderr_redirection_stop(struct ncsh_Output_Redirect_IO* io) {
	dup2(io->original_stderr, STDERR_FILENO);
}

void ncsh_stdout_and_stderr_redirection_start(char* file, struct ncsh_Output_Redirect_IO* io) {
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

void ncsh_stdout_and_stderr_redirection_stop(struct ncsh_Output_Redirect_IO* io) {
	dup2(io->original_stdout, STDOUT_FILENO);
	dup2(io->original_stderr, STDERR_FILENO);
}

void ncsh_stdin_redirection_start(char* file, struct ncsh_Input_Redirect_IO* io) {
	assert(file != NULL);

	int file_descriptor = open(file, O_RDONLY);
	if (file_descriptor == -1) {
		io->fd = -1;
	}

	io->original_stdin = dup(STDIN_FILENO);
	dup2(file_descriptor, STDIN_FILENO);

	close(file_descriptor);
}

void ncsh_stdin_redirection_stop(int original_stdin) {
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

	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

int_fast32_t ncsh_fork_failure(uint_fast32_t command_position, uint_fast32_t number_of_commands, struct ncsh_Pipe_IO* pipes) {
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

	return NCSH_COMMAND_SYNTAX_ERROR;
}

int_fast32_t ncsh_tokenize(struct ncsh_Args* args, struct ncsh_Tokens* tokens) {
	if (args->ops[0] == OP_PIPE)
		return ncsh_syntax_error("ncsh: Invalid syntax: found pipe ('|') as first argument. Correct usage of pipes is 'ls | sort'.\n", 97);
	else if (args->ops[args->count - 1] == OP_PIPE)
		return ncsh_syntax_error("ncsh: Invalid syntax: found pipe ('|') as last argument. Correct usage of pipes is 'ls | sort'.\n", 96);

	for (uint_fast32_t i = 0; i < args->count; ++i) {
		if (args->ops[i] == OP_STDOUT_REDIRECTION) {
			if (i + 1 >= args->count) {
				return ncsh_syntax_error("ncsh: Invalid syntax: found no filename after output redirect symbol '>'.\n", 74);
			}

			tokens->output_file = args->values[i + 1];
			tokens->output_redirect_found_index = i;
		}
		else if (args->ops[i] == OP_STDIN_REDIRECTION) {
			if (i == 0 || i + 1 >= args->count) {
				return ncsh_syntax_error("ncsh: Invalid syntax: found no filename before input redirect symbol '<'.\n", 74);
			}

			tokens->input_file = args->values[i + 1];
			tokens->input_redirect_found_index = i;
		}
		else if (args->ops[i] == OP_PIPE) {
			++tokens->number_of_pipe_commands;
		}
	}
	++tokens->number_of_pipe_commands;

	if (tokens->output_redirect_found_index && tokens->input_redirect_found_index) {
		return ncsh_syntax_error("ncsh: Invalid syntax: found both input and output redirects symbols ('<' and '>', respectively).\n", 97);
	}

	return NCSH_COMMAND_SUCCESS_CONTINUE;
}

int_fast32_t ncsh_vm(struct ncsh_Args* args) {
	pid_t pid = 0;
	int status;
	struct ncsh_Vm vm = {0};
	char* buffer[MAX_INPUT] = {0};
	bool end = false;
	enum ncsh_Ops op_current = OP_NONE;

	struct ncsh_Tokens tokens = {0};
	int_fast32_t tokenize_result = ncsh_tokenize(args, &tokens);
	if (tokenize_result != NCSH_COMMAND_SUCCESS_CONTINUE)
		return tokenize_result;

	uint_fast32_t command_position = 0;
	uint_fast32_t args_position = 0;
	uint_fast32_t buffer_position = 0;

	if (tokens.output_file && tokens.output_redirect_found_index) {
		free(args->values[tokens.output_redirect_found_index]);
		args->values[tokens.output_redirect_found_index] = NULL;
		ncsh_stdout_redirection_start(tokens.output_file, &vm.output_redirect_io);
		if (vm.output_redirect_io.fd == -1) {
			printf("ncsh: Invalid file handle '%s': could not open file for output redirection, do you have permission to open the file?\n",
	  			tokens.output_file);
			return NCSH_COMMAND_FAILED_CONTINUE;
		}
	}
	else if (tokens.input_file && tokens.input_redirect_found_index) {
		free(args->values[tokens.input_redirect_found_index]);
		args->values[tokens.input_redirect_found_index] = NULL;
		ncsh_stdin_redirection_start(tokens.input_file, &vm.input_redirect_io);
		if (vm.input_redirect_io.fd == -1) {
			printf("ncsh: Invalid file handle '%s': could not open file for input redirection, does the file exist?\n",
				tokens.input_file);
			return NCSH_COMMAND_FAILED_CONTINUE;
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

		if (!end)
			op_current = args->ops[args_position];

		buffer[buffer_position] = NULL;
		if (buffer[0] == NULL)
			return NCSH_COMMAND_FAILED_CONTINUE;

		++args_position;

		if (op_current == OP_PIPE && !end) {
			if (!ncsh_pipe_start(command_position, &vm.pipes_io))
				return NCSH_COMMAND_EXIT_FAILURE;
		}

		/*if (ncsh_vm_signal_forward(SIGINT) ||
			ncsh_vm_signal_forward(SIGHUP) ||
			ncsh_vm_signal_forward(SIGTERM) ||
			ncsh_vm_signal_forward(SIGQUIT) ||
			ncsh_vm_signal_forward(SIGUSR1) ||
			ncsh_vm_signal_forward(SIGUSR2)) {
			perror("ncsh: Error setting up signal handlers");
			return NCSH_COMMAND_EXIT_FAILURE;
		}*/
		if (ncsh_vm_signal_forward(SIGINT)) {
			perror("ncsh: Error setting up signal handlers");
			return NCSH_COMMAND_EXIT_FAILURE;
		}

		pid = fork();

		if (pid == -1)
			return ncsh_fork_failure(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);

		int execvp_result = 0;
		if (pid == 0) {
			if (op_current == OP_PIPE)
				ncsh_pipe_connect(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);

			/*if (setpgid(pid, pid) == 0)
				perror(RED "ncsh: Error setting up process group ID for child process" RESET);*/

			if (execvp(buffer[0], buffer) == -1) {
				end = true;
				perror(RED "ncsh: Could not find command or directory" RESET);
				fflush(stdout);
				kill(getpid(), SIGTERM);
				execvp_result = -1;
			}
		}

		if (op_current == OP_PIPE)
			ncsh_pipe_stop(command_position, tokens.number_of_pipe_commands, &vm.pipes_io);

		ncsh_set_child_pid(pid);

		if (execvp_result != 0)
			break;

		int result;
		while (1) {
			status = 0;
			result = waitpid(pid, &status, WUNTRACED);

			// check for errors
			if (result == -1) {
				/* ignore EINTR, occurs when SA_RESTART is not specified in sigaction flags */
				if (errno == EINTR)
					continue;

				perror(RED "ncsh: Error waiting for child process to exit" RESET);
				status = EXIT_FAILURE;
				break;
			}

			// check if child process has exited
			if (result == pid) {
				#ifdef NCSH_DEBUG
				if (WIFEXITED(status)) {
					if (WEXITSTATUS(status))
						fprintf(stderr, "ncsh: Command child process failed with status %d\n", WEXITSTATUS(status));
					else
						fprintf(stderr, "ncsh: Command child process exited successfully.\n");
				}
				else if (WIFSIGNALED(status)) {
					fprintf(stderr, "ncsh: Command child process died from signal #%d\n", WTERMSIG(status));
				}
				else {
					if (write(STDERR_FILENO, "ncsh: Command child process died, cause unknown.\n", 49) == -1) {
						perror("ncsh: Error writing to stderr");
					}
				}
				#endif /* ifdef NCSH_DEBUG */

				break;
			}
		}

		++command_position;
	}

	if (tokens.output_file && tokens.output_redirect_found_index)
		ncsh_stdout_redirection_stop(&vm.output_redirect_io);
	else if (tokens.input_file && tokens.input_redirect_found_index)
		ncsh_stdin_redirection_stop(vm.input_redirect_io.original_stdin);

	if (status == EXIT_FAILURE)
		return NCSH_COMMAND_EXIT_FAILURE;

	return NCSH_COMMAND_SUCCESS_CONTINUE;
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

