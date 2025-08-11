/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.h: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#pragma once

#include "../../types.h"
#include "../statements.h"

int vm_execute(Statements* restrict stmts, Shell* restrict shell, Arena* restrict scratch);

int vm_execute_noninteractive(Statements* restrict stmts, Shell* restrict shell);
