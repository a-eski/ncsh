/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins for ncsh */

#pragma once

#include "vm_types.h"
#include "../arena.h"
#include "../types.h"

bool builtins_check_and_run(Vm_Data* rst vm, Shell* rst shell, Arena* rst scratch_arena);
