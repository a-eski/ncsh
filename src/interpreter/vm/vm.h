/* Copyright ncsh (C) by Alex Eski 2024 */
/* vm.h: the VM for ncsh. Accepts op bytecodes and constant values and their lengths,
 * and processes those into commands. */

#pragma once

#include "../../types.h"
#include "vm_types.h"

int vm_execute(Tokens* rst toks, Shell* rst shell, Arena* rst scratch);

int vm_execute_noninteractive(Tokens* rst toks, Shell* rst shell);
