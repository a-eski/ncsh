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

#include "interpreter/interpreter.c"
#include "interpreter/lexer.c"
#include "interpreter/lexer_op.c"
#include "interpreter/logic.c"
#include "interpreter/parser.c"
#include "interpreter/tokens.c"

#include "readline/ac.c"
#include "readline/hashset.c"
#include "readline/history.c"
#include "readline/ncreadline.c"
#include "readline/terminal.c"
#include "readline/prompt.c"

#include "interpreter/semantic_analyzer.c"
#include "interpreter/vars.c"
#include "interpreter/vm/builtins.c"
#include "interpreter/vm/pipe.c"
#include "interpreter/vm/redirection.c"
#include "interpreter/vm/vm.c"
#include "interpreter/vm/vm_buffer.c"

#include "main.c"
#include "noninteractive.c"
