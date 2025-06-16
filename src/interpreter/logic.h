/* Copyright ncsh (C) by Alex Eski 2025 */
/* logic.h: Preprocessing logic/control flow structures specifically to ensure ready for VM to process. */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "../eskilib/str.h"
#include "tokens.h"
#include "vm/vm_types.h"

enum Logic_Type {
    LT_NONE,
    LT_CODE,
    LT_IF,
    LT_IF_ELSE
};

union Logic_Value {
    int code;
    Token* tok;
};

typedef struct {
    enum Logic_Type type;
    union Logic_Value val;
} Logic_Result;

Logic_Result logic_preprocess(Token* tok, Token_Data* rst data, Arena* rst scratch);
