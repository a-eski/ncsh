/* Copyright ncsh (C) by Alex Eski 2025 */
/* logic.h: Preprocessing logic/control flow structures specifically to ensure ready for VM to process. */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"
#include "vm/vm_types.h"

Logic_Result logic_preprocess(Token* tok, Token_Data* rst data, Arena* rst scratch);
