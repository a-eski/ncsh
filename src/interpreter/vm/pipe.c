/* Copyright ncsh (C) by Alex Eski 2025 */
/* pipe.c: Pipes functions */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../eskilib/ecolors.h"
#include "vm_types.h"

extern int vm_output_fd;

[[nodiscard]]
int pipe_start(size_t command_position, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (command_position % 2 != 0) {
        if (pipe(pipes->fd_one) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return EXIT_FAILURE;
        }
        vm_output_fd = pipes->fd_one[1];
    }
    else {
        if (pipe(pipes->fd_two) != 0) {
            perror(RED "ncsh: Error when piping process" RESET);
            fflush(stdout);
            return EXIT_FAILURE;
        }
        vm_output_fd = pipes->fd_two[1];
    }

    return EXIT_SUCCESS;
}

void pipe_connect(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (!command_position) { // first command
        dup2(pipes->fd_two[1], STDOUT_FILENO);
    }
    else if (command_position == number_of_commands - 1) { // last command
        if (number_of_commands % 2 != 0) {
            dup2(pipes->fd_one[0], STDIN_FILENO);
        }
        else {
            dup2(pipes->fd_two[0], STDIN_FILENO);
        }
    }
    else { // middle command
        if (command_position % 2 != 0) {
            dup2(pipes->fd_two[0], STDIN_FILENO);
            dup2(pipes->fd_one[1], STDOUT_FILENO);
            vm_output_fd = pipes->fd_one[1];
        }
        else {
            dup2(pipes->fd_one[0], STDIN_FILENO);
            dup2(pipes->fd_two[1], STDOUT_FILENO);
            vm_output_fd = pipes->fd_two[1];
        }
    }
}

void pipe_stop(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes)
{
    assert(pipes);

    if (!command_position) {
        close(pipes->fd_two[1]);
    }
    else if (command_position == number_of_commands - 1) {
        if (number_of_commands % 2 != 0) {
            close(pipes->fd_one[0]);
            vm_output_fd = STDOUT_FILENO;
        }
        else {
            close(pipes->fd_two[0]);
            vm_output_fd = STDOUT_FILENO;
        }
    }
    else {
        if (command_position % 2 != 0) {
            close(pipes->fd_two[0]);
            close(pipes->fd_one[1]);
            vm_output_fd = STDOUT_FILENO;
        }
        else {
            close(pipes->fd_one[0]);
            close(pipes->fd_two[1]);
            vm_output_fd = STDOUT_FILENO;
        }
    }
}
