/* Copyright ncsh (C) by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "interpreter/interpreter.h"
#include "noninteractive.h"
#include "ttyterm/ttyterm.h"
#include "shell.h"
#include "debug.h"

/* noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see defines.h (EXIT_...)
 */
[[nodiscard]]
int noninteractive(int argc, char** rst argv)
{
    assert(argc > 1); // 1 because first arg is ncsh
    assert(argv);
    assert(!strcmp("ncsh", argv[0]) || !strcmp("./bin/ncsh", argv[0]));

    // noninteractive does not support no args
    if (argc <= 1) {
        return EXIT_FAILURE;
    }

    debug("ncsh running in noninteractive mode.");
    debug_argsv(argc, argv);

    constexpr int arena_capacity = 1 << 16;
    char* memory = malloc(arena_capacity);
    if (!memory) {
        term_color_set(TERM_RED_ERROR);
        term_puts("ncsh: could not start up, not enough memory available.");
        term_color_reset();
        return EXIT_FAILURE;
    }

    Shell shell = {0};
    shell.arena = (Arena){.start = memory, .end = memory + (arena_capacity)};

    if (config_init(&shell.config, &shell.arena, shell.arena) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    interpreter_init(&shell);

    int exit_code = interpreter_run_noninteractive(argv + 1, (size_t)argc - 1, &shell);

    free(memory);

    return exit_code;
}
