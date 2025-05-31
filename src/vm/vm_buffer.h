/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_buffer.c: setup commands in a form that can be sent to execvp/execve/etc. */

#include "../args.h"
#include "vm_types.h"
#include <stdint.h>


Arg* vm_buffer_set(Arg* rst arg, Token_Data* rst tokens, Vm_Data* rst vm);
