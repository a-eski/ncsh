/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdint.h>

#include "../ncsh_parser.h"
#include "../ncsh_types.h"

int_fast32_t ncsh_vm_execute(struct ncsh_Shell* const restrict shell,
                             struct ncsh_Arena* const scratch_arena);

int_fast32_t ncsh_vm_execute_noninteractive(struct ncsh_Args* const restrict args,
                                            struct ncsh_Arena* const arena);
