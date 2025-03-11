// Copyright (c) ncsh by Alex Eski 2024

#ifndef NCSH_VM_H_
#define NCSH_VM_H_

#include <stdint.h>

#include "../ncsh_parser.h"
#include "../ncsh_types.h"

int_fast32_t ncsh_vm_execute(struct ncsh_Shell* const restrict shell);

int_fast32_t ncsh_vm_execute_noninteractive(struct ncsh_Args* const restrict args);

#endif // !NCSH_VM_H_
