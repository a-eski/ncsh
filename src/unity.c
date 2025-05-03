/* Copyright ncsh by Alex Eski 2024 */

/* Used to build ncsh with a single translation unit, aka a unity build.
 * Try it out with 'make unity_debug' or 'make unity'.
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include "eskilib/efile.c"

#include "z/fzf.c"
#include "z/z.c"

#include "arena.c"
#include "args.c"
#include "config.c"
#include "main.c"
#include "noninteractive.c"
#include "parser.c"
#include "readline/ac.c"
#include "readline/hashset.c"
#include "readline/history.c"
#include "readline/ncreadline.c"
#include "readline/terminal.c"
#include "vars.c"
#include "vm/vm.c"
#include "vm/vm_builtins.c"
#include "vm/vm_tokenizer.c"
