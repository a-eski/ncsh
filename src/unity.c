// Copyright (c) ncsh by Alex Eski 2024

// Used to build ncsh with a single translation unit, aka a unity build.
// Try it out with 'make unity_debug' or 'make unity'.

#define _POSIX_C_SOURCE 200809L

#include "eskilib/eskilib_file.c"
#include "eskilib/eskilib_hashtable.c"
#include "eskilib/eskilib_string.c"

#include "z/fzf.c"
#include "z/z.c"

#include "main.c"
#include "ncsh.c"
#include "ncsh_autocompletions.c"
#include "ncsh_builtins.c"
#include "ncsh_config.c"
#include "ncsh_history.c"
#include "ncsh_noninteractive.c"
#include "ncsh_parser.c"
#include "ncsh_terminal.c"
#include "ncsh_interpreter.c"
#include "ncsh_vm.c"
