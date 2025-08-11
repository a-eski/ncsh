/* Copyright ncsh (C) by Alex Eski 2024 */
/* builtins.h: shell builtins for ncsh */

#pragma once

#include "../../arena.h"
#include "../../types.h"
#include "vm_types.h"

bool builtins_check_and_run(Vm_Data* restrict vm, Shell* restrict shell, Arena* restrict scratch_arena);
