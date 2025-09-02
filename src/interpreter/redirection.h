/* Copyright ncsh (C) by Alex Eski 2025 */
/* vm_redirection.h: IO Redirection */

#pragma once

#include "vm_types.h"

int redirection_start_if_needed(Vm_Data* restrict vm);

void redirection_stop_if_needed(Vm_Data* restrict vm);
