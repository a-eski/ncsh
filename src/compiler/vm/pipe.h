/* Copyright ncsh (C) by Alex Eski 2025 */
/* pipe.h: Pipes functions */

#include <stddef.h>

#include "vm_types.h"

int pipe_start(size_t command_position, Pipe_IO* rst pipes);

void pipe_connect(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes);

void pipe_stop(size_t command_position, size_t number_of_commands, Pipe_IO* rst pipes);
