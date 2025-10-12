/* Copyright ncsh (C) by Alex Eski 2025 */
/* vars.h: stores variable values in a hash table */

#pragma once

#include "eskilib/str.h"
#include "types.h"

void vars_new(Shell* restrict shell);

Var* vars_add_or_get(Vars* vars, Str key);
