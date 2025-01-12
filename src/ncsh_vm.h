// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_vm_h
#define ncsh_vm_h

#include <stdint.h>

#include "ncsh_parser.h"

int_fast32_t ncsh_vm_execute(struct ncsh_Args *args);

int_fast32_t ncsh_vm_execute_noninteractive(struct ncsh_Args *args);

#endif // !ncsh_vm_h

