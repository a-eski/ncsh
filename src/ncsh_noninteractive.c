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

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_debug.h"
#include "ncsh_noninteractive.h"
#include "ncsh_parser.h"
#include "vm/ncsh_vm.h"

/* ncsh_noninteractive_run
 * Parses and sends output of parser to VM. Parser data stored in scratch arena, which is then used by VM.
 * Scratch arena reset after scope ends due to passing by value.
 */
int_fast32_t ncsh_noninteractive_run(const char** const restrict argv,
                                     const size_t argc,
                                     struct ncsh_Args* args,
                                     struct ncsh_Arena* const arena,
                                     struct ncsh_Arena scratch_arena)
{
    ncsh_parser_parse_noninteractive(argv, argc, args, &scratch_arena);

    return ncsh_vm_execute_noninteractive(args, arena);
}

/* ncsh_noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see ncsh_defines.h (EXIT_...)
 */
[[nodiscard]]
int_fast32_t ncsh_noninteractive(const int argc,
                                 const char** const restrict argv)
{
    assert(argc > 1); // 1 because first arg is ncsh
    assert(argv);
    // noninteractive does not support no args
    if (argc <= 1) {
        return EXIT_FAILURE;
    }

#ifdef NCSH_DEBUG
    printf("ncsh running in noninteractive mode.\n");
#endif /* ifdef NCSH_DEBUG */
    constexpr int arena_capacity = 1<<16;
    constexpr int scratch_arena_capacity = 1<<16;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return EXIT_FAILURE;
    }

    struct ncsh_Arena arena = { .start = memory, .end = memory + (arena_capacity) };
    char* scratch_memory_start = memory + (arena_capacity + 1);
    struct ncsh_Arena scratch_arena = { .start = scratch_memory_start, .end = scratch_memory_start + (scratch_arena_capacity) };

    struct ncsh_Config config;
    if (ncsh_config_init(&config, &arena, scratch_arena) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    struct ncsh_Args args = {0};
    enum eskilib_Result result;
    if ((result = ncsh_parser_args_alloc(&args, &arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

#ifdef NCSH_DEBUG
    ncsh_debug_argsv(argc, argv);
#endif /* ifdef NCSH_DEBUG */

    int_fast32_t command_result = ncsh_noninteractive_run(argv + 1, (size_t)argc - 1, &args, &arena, scratch_arena);

    int_fast32_t exit_code = command_result == NCSH_COMMAND_EXIT_FAILURE
        ? EXIT_FAILURE
        : EXIT_SUCCESS;

    free(memory);

    return exit_code;
}
