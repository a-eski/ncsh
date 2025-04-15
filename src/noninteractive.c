/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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
#include "vm/vm.h"

/* noninteractive_run
 * Parses and sends output of parser to VM. Parser data stored in scratch arena, which is then used by VM.
 * Scratch arena reset after scope ends due to passing by value.
 */
int_fast32_t noninteractive_run(const char** const restrict argv, const size_t argc, struct Args* args,
                                struct Arena* const arena, struct Arena scratch_arena)
{
    parser_parse_noninteractive(argv, argc, args, arena, &scratch_arena);

    return vm_execute_noninteractive(args, arena);
}

/* noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see defines.h (EXIT_...)
 */
[[nodiscard]]
int_fast32_t noninteractive(const int argc, const char** const restrict argv)
{
    assert(argc > 1); // 1 because first arg is ncsh
    assert(argv);
    assert(!strcmp("ncsh", argv[0]));

    // noninteractive does not support no args
    if (argc <= 1) {
        return EXIT_FAILURE;
    }

    debug("ncsh running in noninteractive mode.");

    constexpr int arena_capacity = 1 << 16;
    constexpr int scratch_arena_capacity = 1 << 16;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return EXIT_FAILURE;
    }

    struct Arena arena = {.start = memory, .end = memory + (arena_capacity)};
    char* scratch_memory_start = memory + (arena_capacity + 1);
    struct Arena scratch_arena = {.start = scratch_memory_start,
                                  .end = scratch_memory_start + (scratch_arena_capacity)};

    struct Config config;
    if (config_init(&config, &arena, scratch_arena) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    struct Args args = {0};
    enum eresult result;
    if ((result = parser_init(&args, &arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    debug_argsv(argc, argv);

    int_fast32_t command_result = noninteractive_run(argv + 1, (size_t)argc - 1, &args, &arena, scratch_arena);

    int_fast32_t exit_code = command_result == NCSH_COMMAND_EXIT_FAILURE ? EXIT_FAILURE : EXIT_SUCCESS;

    free(memory);

    return exit_code;
}
