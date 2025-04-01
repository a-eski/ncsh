/* Copyright ncsh by Alex Eski 2024 */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <readline/ncsh_autocompletions.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "readline/ncsh_history.h"
#include "readline/ncsh_input.h"
#include "ncsh_types.h"
#include "vm/ncsh_vm.h"

/* ncsh_exit
 * Called on exit to free memory related to the shells lifetime & the shells main loops lifetime.
 */
void ncsh_exit(struct ncsh_Shell* const restrict shell)
{
    if (shell->input.buffer && *shell->input.buffer) {
        free(shell->input.buffer);
    }
    ncsh_history_save(&shell->input.history);
    z_exit(&shell->z_db);
}

[[nodiscard]]
char* ncsh_init_arena(struct ncsh_Shell* const restrict shell)
{
    constexpr int arena_capacity = 1<<24;
    constexpr int scratch_arena_capacity = 1<<16;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        return NULL;
    }

    shell->arena = (struct ncsh_Arena){ .start = memory, .end = memory + (arena_capacity) };
    char* scratch_memory_start = memory + (arena_capacity + 1);
    shell->scratch_arena = (struct ncsh_Arena){ .start = scratch_memory_start, .end = scratch_memory_start + (scratch_arena_capacity) };

    return memory;
}

/* ncsh_init
 * Called on startup to allocate memory related to the shells lifetime.
 * Returns: exit result, EXIT_SUCCESS or EXIT_FAILURE
 */
[[nodiscard]]
char* ncsh_init(struct ncsh_Shell* const restrict shell)
{
    char* memory = ncsh_init_arena(shell);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return NULL;
    }

    if (ncsh_config_init(&shell->config, &shell->arena, shell->scratch_arena) != E_SUCCESS) {
        return NULL;
    }

    if (ncsh_input_init(&shell->config, &shell->input, &shell->arena) != EXIT_SUCCESS) {
        return NULL;
    }

    enum eskilib_Result result;
    if ((result = ncsh_parser_args_alloc(&shell->args, &shell->arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        return NULL;
    }

    enum z_Result z_result = z_init(&shell->config.config_location, &shell->z_db, &shell->arena);
    if (z_result != Z_SUCCESS) {
        return NULL;
    }

    return memory;
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
 * 3. ncsh_input has its own inner lifetime via the scratch arena.
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

    char* memory = ncsh_init(&shell);
    if (!memory) {
        return EXIT_FAILURE;
    }

#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);
#endif

    int_fast32_t exit_code = EXIT_SUCCESS;

    while (1) {
        int_fast32_t input_result = ncsh_input(&shell.input, shell.scratch_arena);
        switch (input_result) {
        case EXIT_FAILURE: {
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        /*case EXIT_SUCCESS: {
            goto reset;
        }*/
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

        ncsh_history_add(shell.input.buffer, shell.input.pos, &shell.input.history, &shell.arena);
        ncsh_autocompletions_add(shell.input.buffer, shell.input.pos, shell.input.autocompletions_tree, &shell.arena);

    reset:
        if (shell.input.buffer && *shell.input.buffer) {
            free(shell.input.buffer);
        }
        shell.input.pos = 0;
        shell.args.count = 0;
        shell.args.values[0] = NULL;
    }

exit:
    ncsh_exit(&shell);
    free(memory);

    return exit_code;
}
