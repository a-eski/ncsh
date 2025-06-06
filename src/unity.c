/* Copyright ncsh by Alex Eski 2024 */

/* Used to build ncsh with a single translation unit, aka a unity build.
 * Try it out with 'make unity_debug' or 'make unity'.
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#ifdef NCSH_DEBUG
#include "debug.h"
#endif /* ifdef NCSH_DEBUG */

#include "eskilib/efile.c"

#include "z/fzf.c"
#include "z/z.c"

#include "alias.c"
#include "arena.c"
#include "args.c"
#include "config.c"
#include "env.c"
#include "parser.c"

#include "main.c"
#include "noninteractive.c"

#include "readline/ac.c"
#include "readline/hashset.c"
#include "readline/history.c"
#include "readline/ncreadline.c"
#include "readline/terminal.c"

#include "vm/builtins.c"
#include "vm/logic.c"
#include "vm/pipe.c"
#include "vm/preprocessor.c"
#include "vm/redirection.c"
#include "vm/syntax_validator.c"
#include "vm/vars.c"
#include "vm/vm.c"
#include "vm/vm_buffer.c"
