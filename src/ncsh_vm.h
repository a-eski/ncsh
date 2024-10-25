// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_vm_h
#define ncsh_vm_h

#include <stdint.h>

#include "ncsh_history.h"

uint_fast32_t ncsh_vm_execute(struct ncsh_Args args, struct ncsh_History* history);

#endif // !ncsh_vm_h

