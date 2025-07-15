/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins for ncsh */

#pragma once

#include "../../arena.h"
#include "../../shell.h"
#include "vm_types.h"

bool builtins_check_and_run(Vm_Data* rst vm, Shell* rst shell, Arena* rst scratch_arena);
