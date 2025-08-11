/* Copyright ncsh (C) by Alex Eski 2025 */
/* io.h: read input, handle prompts, history, and autocompletions */

#pragma once

#include <stddef.h>

#include "../types.h"

#define LINE_LIMIT 100

/* io_init
 * Loads history and autocompletions.
 * input. Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int io_init(Config* restrict config, Env* restrict env, Input* restrict input, Arena* restrict arena);

/* io_readline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and
 * other inputs. Accepts a pointer to the scratch arena, but it passes a copy (by value) to autocompletion logic when it
 * is needed to be used. Returns: exit status( EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...))
 */
int io_readline(Input* restrict input, Arena* restrict scratch_arena);

/* io_deinit
 * Saves history changes
 */
void io_deinit(Input* restrict input, Arena scratch);
