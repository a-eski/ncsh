/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.h: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#pragma once

#include "../types.h"

int vm_execute(Args* rst args, Shell* rst shell, Arena* rst scratch_arena);

int vm_execute_noninteractive(Args* rst args, Shell* rst shell);
