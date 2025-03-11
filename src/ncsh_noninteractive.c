// Copyright (c) ncsh by Alex Eski 2024

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
#include "ncsh_defines.h"
#include "ncsh_debug.h"
#include "ncsh_noninteractive.h"
#include "ncsh_parser.h"
#include "vm/ncsh_vm.h"

/* ncsh_noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_noninteractive(int argc, char** argv)
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

    struct ncsh_Args args = {0};
    enum eskilib_Result result;
    if ((result = ncsh_parser_args_malloc(&args)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        if (result != E_FAILURE_MALLOC) {
            ncsh_parser_args_free(&args);
        }
        return EXIT_FAILURE;
    }

#ifdef NCSH_DEBUG
    ncsh_debug_argsv(argc, argv);
#endif /* ifdef NCSH_DEBUG */

    ncsh_parser_parse_noninteractive(argv + 1, (size_t)argc - 1, &args); // argv + 1 because ncsh is first argv

    int_fast32_t command_result = ncsh_vm_execute_noninteractive(&args);

    int_fast32_t exit_code = command_result == NCSH_COMMAND_EXIT_FAILURE
        ? EXIT_FAILURE
        : EXIT_SUCCESS;

    ncsh_parser_args_free_values(&args);
    ncsh_parser_args_free(&args);

    return exit_code;
}
