/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_math.h: math processing */

#pragma once

#include "parse.h"
#include "vm_types.h"

static inline bool vm_is_math_cond(enum Ops op)
{
    return op == OP_EQ_A ||
        op == OP_GT_A || op == OP_GE_A ||
        op == OP_LT_A || op == OP_LE_A;
}

void vm_math_condition(Vm_Data* restrict vm);

Str vm_math_expr(Vm_Data* restrict vm);
