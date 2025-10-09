/* Copyright ncsh (C) by Alex Eski 2025 */

#pragma once

#include "stmts.h"
#include "vm_types.h"


void expand_expr_variables(Commands* restrict cmds, size_t p, Env* restrict env, Arena* restrict scratch);

void expand_assignment(Commands* restrict cmds, Shell* restrict shell);

void expand(Vm_Data* restrict vm, Arena* restrict scratch);
