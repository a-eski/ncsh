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
#include "eskilib/ecolors.h"
#include "eskilib/eresult.h"
#include "noninteractive.h"
#include "parser.h"
#include "vars.h"
#include "vm/vm.h"

/* noninteractive_run
 * Parses and sends output of parser to VM. Parser data stored in scratch arena, which is then used by VM.
 * Scratch arena reset after scope ends due to passing by value.
 */
int noninteractive_run(char** restrict argv, size_t argc, struct Shell* shell)
{
    struct Args* args = parser_parse_noninteractive(argv, argc, &shell->arena);

    return vm_execute_noninteractive(args, shell);
}

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

    constexpr int arena_capacity = 1 << 16;
    char* memory = malloc(arena_capacity);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return EXIT_FAILURE;
    }

    struct Shell shell = {0};
    shell.arena = (struct Arena){.start = memory, .end = memory + (arena_capacity)};

    if (config_init(&shell.config, &shell.arena) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    vars_malloc(&shell.arena, &shell.vars);

    debug_argsv(argc, argv);

    int command_result = noninteractive_run(argv + 1, (size_t)argc - 1, &shell);

    int exit_code = command_result == NCSH_COMMAND_EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;

    free(memory);

    return exit_code;
}
