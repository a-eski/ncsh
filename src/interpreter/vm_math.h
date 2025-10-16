/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_math.h: math processing */

#pragma once

#include "parse.h"
#include "vm_types.h"

static inline bool vm_is_math_cond(enum Ops op)
{
    return op == OP_EQUALS ||
        op == OP_GREATER_THAN || op == OP_GREATER_THAN_OR_EQUALS ||
        op == OP_LESS_THAN || op == OP_LESS_THAN_OR_EQUALS;
}

void vm_math_condition(Vm_Data* restrict vm);

Str vm_math_expr(Vm_Data* restrict vm);
