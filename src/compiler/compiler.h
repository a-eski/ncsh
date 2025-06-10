/* Copyright ncsh (C) by Alex Eski 2025 */
/* compiler.h: compiler interface for ncsh */

#pragma once

#include "../types.h"

void compiler_init(Shell* rst shell);

/* compiler_run
 * Lex, parse, perform semantic analysis, and execute.
 * Pass in copy of scratch arena so it is valid for scope of compiler, then resets when scope ends.
 * The scratch arena needs to be valid for scope of compiler_run, since values are stored in the scratch arena.
 * Do not change scratch arena to Arena*.
 */
int compiler_run(Shell* rst shell, Arena scratch);

/* compiler_run_noninteractive
 * Lex, parse, perform semantic analysis, and execute.
 * Does not need the scratch arena, since noninteractive has very straghtforward liftime.
 */
int compiler_run_noninteractive(char** rst argv, size_t argc, Shell* rst shell);
