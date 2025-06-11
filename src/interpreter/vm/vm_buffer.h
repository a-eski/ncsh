/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_buffer.c: setup commands in a form that can be sent to execvp/execve/etc. */

#include <stdint.h>

#include "vm_types.h"

Token* vm_buffer_set(Token* rst tok, Token_Data* rst data, Vm_Data* rst vm);
