/* Copyright ncsh (C) by Alex Eski 2025 */
/* vars.h: stores variable values in a hash table */

#pragma once

#include "eskilib/str.h"
#include "types.h"

#define Var_n(n) (Var){.type = V_NUM, .val.n = n}

#define Var_s(str) (Var){.type = V_STR, .val.s = str}

void vars_new(Shell* restrict shell);

Var* vars_add_or_get(Vars* vars, Str key);
