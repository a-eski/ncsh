/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdint.h>
#include <fcntl.h>

#include "../parser.h"
#include "../types.h"

inline int vm_output_redirection_oflags_get(const bool append)
{
    return append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT | O_TRUNC;
}

int_fast32_t vm_execute(struct Shell* const restrict shell, struct Arena* const scratch_arena);

int_fast32_t vm_execute_noninteractive(struct Args* const restrict args, struct Arena* const arena);
