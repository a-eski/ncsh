/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.h: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#pragma once

#include <fcntl.h>

#include "../parser.h"
#include "../types.h"

inline int vm_output_redirection_oflags_get(const bool append)
{
    return append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
}

int vm_execute(struct Shell* const restrict shell, struct Arena* const scratch_arena);

int vm_execute_noninteractive(struct Args* const restrict args, struct Arena* const arena);
