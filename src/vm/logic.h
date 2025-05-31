/* Copyright ncsh (C) by Alex Eski 2025 */
/* logic.h: Preprocessing logic/control flow structures specifically to ensure ready for VM to process. */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"
#include "vm_types.h"

Logic_Result logic_preprocess(Arg* arg, Token_Data* rst tokens, Arena* rst scratch);
