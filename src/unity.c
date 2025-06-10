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
#include "config.c"
#include "env.c"

#include "compiler/compiler.c"
#include "compiler/lexer.c"
#include "compiler/logic.c"
#include "compiler/parser.c"
#include "compiler/tokens.c"

#include "readline/ac.c"
#include "readline/hashset.c"
#include "readline/history.c"
#include "readline/ncreadline.c"
#include "readline/terminal.c"

#include "compiler/vm/builtins.c"
#include "compiler/vm/pipe.c"
#include "compiler/vm/redirection.c"
#include "compiler/vm/syntax_validator.c"
#include "compiler/vm/vars.c"
#include "compiler/vm/vm.c"
#include "compiler/vm/vm_buffer.c"

#include "main.c"
#include "noninteractive.c"
