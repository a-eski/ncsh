/* Copyright ncsh (C) by Alex Eski 2025 */
/* interpreter.h: interpreter interface for ncsh */

#pragma once

#include "../shell.h"

void interpreter_init(Shell* rst shell);

/* interpreter_run
 * Lex, parse, perform semantic analysis, and execute.
 * Pass in copy of scratch arena so it is valid for scope of interpreter, then resets when scope ends.
 * The scratch arena needs to be valid for scope of interpreter_run, since values are stored in the scratch arena.
 * Do not change scratch arena to Arena*.
 */
int interpreter_run(Shell* rst shell, Arena scratch);

/* interpreter_run_noninteractive
 * Lex, parse, perform semantic analysis, and execute.
 * Does not need the scratch arena, since noninteractive has very straghtforward liftime.
 */
int interpreter_run_noninteractive(char** rst argv, size_t argc, Shell* rst shell);
