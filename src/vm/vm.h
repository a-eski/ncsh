/* Copyright ncsh by Alex Eski 2024 */

#pragma once

#include <stdint.h>

#include "../parser.h"
#include "../types.h"

void vm_init(struct Shell* const restrict shell, struct Arena* const arena);

int_fast32_t vm_execute(struct Shell* const restrict shell, struct Arena* const scratch_arena);

int_fast32_t vm_execute_noninteractive(struct Args* const restrict args, struct Arena* const arena);
