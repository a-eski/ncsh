/* Copyright (c) ncsh by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_parser.h"
#include "readline/ncsh_readline.h"
#include "ncsh_types.h"
#include "vm/ncsh_vm.h"

/* ncsh_exit
 * Called on exit to free memory related to the shells lifetime & the shells main loops lifetime.
 */
void ncsh_exit(struct ncsh_Shell* const restrict shell)
{
    ncsh_readline_exit(&shell->input);
    z_exit(&shell->z_db);
}

/* ncsh_init
 * Called on startup to allocate memory related to the shells lifetime.
 * Returns: exit result, EXIT_SUCCESS or EXIT_FAILURE
 */
[[nodiscard]]
int_fast32_t ncsh_init(struct ncsh_Shell* const restrict shell)
{
    if (ncsh_config_init(&shell->config, &shell->arena, shell->scratch_arena) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    if (ncsh_readline_init(&shell->config, &shell->input, &shell->arena) != EXIT_SUCCESS) {
        ncsh_readline_exit(&shell->input);
        return EXIT_FAILURE;
    }

    enum eskilib_Result result;
    if ((result = ncsh_parser_args_alloc(&shell->args, &shell->arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        ncsh_readline_exit(&shell->input);
        return EXIT_FAILURE;
    }

    enum z_Result z_result = z_init(&shell->config.config_location, &shell->z_db, &shell->arena);
    if (z_result != Z_SUCCESS) {
        ncsh_readline_exit(&shell->input);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* ncsh_reset
 * Manages freeing the lifetime of the main shell loop.
 * Frees memory used by the parser to populate ncsh_Args.
 */
void ncsh_reset(struct ncsh_Shell* const restrict shell)
{
    memset(shell->input.buffer, '\0', shell->input.max_pos);
    shell->input.pos = 0;
    shell->input.max_pos = 0;
    shell->args.count = 0;
    shell->args.values[0] = NULL;
}

/* ncsh_run
 * Parse and execute.
 * Pass in copy of scratch arena so it is valid for scope of parser and VM, then resets when scope ends.
 * The scratch arena needs to be valid for scope of ncsh_vm_execute, since values are stored in the scratch arena.
 * Do not change scratch arena to struct ncsh_Arena* (pointer).
 */
int_fast32_t ncsh_run(struct ncsh_Shell* shell, struct ncsh_Arena scratch_arena)
{
    ncsh_parser_parse(shell->input.buffer, shell->input.pos, &shell->args, &scratch_arena);

    return ncsh_vm_execute(shell, &scratch_arena);
}

/* ncsh
 * Handles initialization, exit, and the main loop of the shell.
 *
 * This function represents interactive mode, for noninteractive see ncsh_noninteractive.
 *
 * Has several lifetimes.
 * 1. The lifetime of the shell, managed through the arena. See ncsh_init and ncsh_exit.
 * 2. The lifetime of the main loop of the shell, managed through the scratch arena.
 * 3. ncsh_readline has its own inner lifetime via the scratch arena.
 */
[[nodiscard]]
int_fast32_t ncsh(void)
{
#ifdef NCSH_START_TIME
    clock_t start = clock();
#endif

#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
    constexpr size_t clear_screen_len = sizeof(CLEAR_SCREEN MOVE_CURSOR_HOME) - 1;
    if (write(STDOUT_FILENO, CLEAR_SCREEN MOVE_CURSOR_HOME, clear_screen_len) == -1) {
        return EXIT_FAILURE;
    }
#endif

    struct ncsh_Shell shell = {0};

    constexpr int arena_capacity = 1<<24;
    constexpr int scratch_arena_capacity = 1<<16;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return EXIT_FAILURE;
    }

    shell.arena = (struct ncsh_Arena){ .start = memory, .end = memory + (arena_capacity) };
    char* scratch_memory_start = memory + (arena_capacity + 1);
    shell.scratch_arena = (struct ncsh_Arena){ .start = scratch_memory_start, .end = scratch_memory_start + (scratch_arena_capacity) };

    if (ncsh_init(&shell) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);
#endif

    int_fast32_t exit_code = EXIT_SUCCESS;

    while (1) {
        int_fast32_t input_result = ncsh_readline(&shell.input, &shell.scratch_arena);
        switch (input_result) {
        case EXIT_FAILURE: {
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        case EXIT_SUCCESS: {
            goto reset;
        }
        case EXIT_SUCCESS_END: {
            goto exit;
        }
        }

        int_fast32_t command_result = ncsh_run(&shell, shell.scratch_arena);
        switch (command_result) {
        case NCSH_COMMAND_EXIT_FAILURE: {
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        case NCSH_COMMAND_EXIT: {
            puts("exit");
            goto exit;
        }
        case NCSH_COMMAND_SYNTAX_ERROR:
        case NCSH_COMMAND_FAILED_CONTINUE: {
            goto reset;
        }
        }

        ncsh_readline_history_and_autocompletion_add(&shell.input, &shell.arena);

    reset:
        ncsh_reset(&shell);
    }

exit:
    ncsh_exit(&shell);
    free(memory);

    return exit_code;
}
