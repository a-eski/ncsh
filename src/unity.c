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

#ifndef TERMINFO_DIRS
#define TERMINFO_DIRS "" // this is defined at compile time.
#endif /* ifndef TERMINFO_DIRS */

#include "ttyio/lib/uniutil.c"
#include "ttyio/lib/uninames.c"
#include "ttyio/lib/unibilium.c"
#include "ttyio/tcaps.c"
#include "ttyio/ttyio.c"

#include "z/fzf.c"
#include "z/z.c"

#include "interpreter/expansions.c"
#include "interpreter/interpreter.c"
#include "interpreter/lexemes.c"
#include "interpreter/lexer.c"
#include "interpreter/parser.c"
#include "interpreter/statements.c"

#include "io/ac.c"
#include "io/hashset.c"
#include "io/history.c"
#include "io/io.c"
#include "io/prompt.c"

#include "interpreter/semantic_analyzer.c"
#include "interpreter/vars.c"
#include "interpreter/vm/builtins.c"
#include "interpreter/vm/pipe.c"
#include "interpreter/vm/redirection.c"
#include "interpreter/vm/vm.c"

#include "alias.c"
#include "arena.c"
#include "config.c"
#include "env.c"

#include "main.c"
#include "noninteractive.c"
