/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_redirection.h: IO Redirection */

#include "vm_types.h"
#include "../statements.h"

int redirection_start_if_needed(Statements* rst statements, Vm_Data* rst vm);

void redirection_stop_if_needed(Vm_Data* rst vm);
