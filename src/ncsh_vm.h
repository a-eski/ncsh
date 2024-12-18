// Copyright (c) ncsh by Alex Eski 2024

#ifndef ncsh_vm_h
#define ncsh_vm_h

#include <stdint.h>

void ncsh_vm_parse(char line[], uint_fast32_t length, struct ncsh_Args* args);

int_fast32_t ncsh_vm_execute(struct ncsh_Args* args);

#endif // !ncsh_vm_h

