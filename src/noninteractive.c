/* Copyright ncsh (C) by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "conf.h"
#include "eskilib/eresult.h"
#include "interpreter/interpreter.h"
#include "noninteractive.h"
#include "ttyio/ttyio.h"
#include "shell.h"
#include "debug.h"

/* noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see defines.h (EXIT_...)
 */
[[nodiscard]]
int noninteractive(int argc, char** restrict argv)
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

    tty_init_caps();

    constexpr int arena_capacity = 1 << 16;
    char* memory = malloc(arena_capacity);
    if (!memory) {
        tty_color_set(TTYIO_RED_ERROR);
        tty_puts("ncsh: could not start up, not enough memory available.");
        tty_color_reset();
        tty_deinit_caps();
        return EXIT_FAILURE;
    }

    Shell shell = {0};
    shell.arena = (Arena){.start = memory, .end = memory + (arena_capacity)};

    int rv = EXIT_SUCCESS;
    if (config_init(&shell.config, &shell.arena, shell.arena) != E_SUCCESS) {
        rv = EXIT_FAILURE;
        goto exit;
    }

    interpreter_init(&shell);

    rv = interpreter_run_noninteractive(argv + 1, (size_t)argc - 1, &shell);

exit:
    tty_deinit_caps();
    free(memory);
    return rv;
}
