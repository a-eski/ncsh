/* Copyright ncsh by Alex Eski 2024 */

/* Used to build ncsh with a single translation unit, aka a unity build.
 * Try it out with 'make unity_debug' or 'make unity'.
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "eskilib/eskilib_file.c"
#include "eskilib/eskilib_hashtable.c"

#include "z/fzf.c"
#include "z/z.c"

#include "main.c"
#include "ncsh_arena.c"
#include "ncsh.c"
#include "ncsh_config.c"
#include "readline/ncsh_terminal.c"
#include "readline/ncsh_readline.c"
#include "readline/ncsh_autocompletions.c"
#include "readline/ncsh_history.c"
#include "ncsh_noninteractive.c"
#include "ncsh_parser.c"
#include "vm/ncsh_vm_builtins.c"
#include "vm/ncsh_vm_tokenizer.c"
#include "vm/ncsh_vm.c"
