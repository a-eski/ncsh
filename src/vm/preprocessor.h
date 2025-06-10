/* Copyright ncsh (C) by Alex Eski 2025 */
/* preprocessor.h: Preprocessing of parser output to ensure ready for VM to process. */

#pragma once

#include "../parser/args.h"
#include "../types.h"
#include "vm_types.h"

int preprocessor_preprocess(Args* rst args, Token_Data* rst tokens, Shell* rst shell, Arena* rst scratch_arena);
