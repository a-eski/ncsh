/* Copyright ncsh (C) by Alex Eski 2024 */

#pragma once

#include <stddef.h>

#include "../arena.h"
#include "args.h"

/* parser_parse
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Returns: The args ready to be processed by VM, allocated with the scratch arena.
 */
Args* parser_parse(char* rst line, size_t length, Arena* rst scratch_arena);

/* parser_parse_noninteractive
 * Turns the inputted line into values, lengths, and bytecodes that the VM can work with.
 * Used for noninteractive mode.
 * Returns: The args ready to be processed by VM, allocated with the scratch arena.
 */
Args* parser_parse_noninteractive(char** rst inputs, size_t inputs_count, Arena* rst scratch_arena);
