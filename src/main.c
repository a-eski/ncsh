/* Copyright ncsh by Alex Eski 2024 */

#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "defines.h"
#include "eskilib/ecolors.h"
#include "eskilib/eresult.h"
#include "interpreter/interpreter.h"
#include "noninteractive.h"
#include "readline/ac.h"
#include "readline/ncreadline.h"
#include "signals.h"

/* Global Variables
 * Globals should be minimized as much as possible.
 * These are only used to simplify signal handling and failure cases.
 */
jmp_buf env;
sig_atomic_t vm_child_pid;
volatile int sigwinch_caught;

/* arena_init
 * Initialize arenas used for the lifteim of the shell.
 * A permanent arena and a scratch arena are allocated.
 * Returns: pointer to start of the memory block allocated.
 */
[[nodiscard]]
char* arena_init(Shell* rst shell)
{
    constexpr int arena_capacity = 1 << 24;
    constexpr int scratch_arena_capacity = 1 << 20;
    constexpr int total_capacity = arena_capacity + scratch_arena_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        return NULL;
    }

    shell->arena = (Arena){.start = memory, .end = memory + (arena_capacity)};
    char* scratch_memory_start = memory + (arena_capacity + 1);
    shell->scratch_arena =
        (Arena){.start = scratch_memory_start, .end = scratch_memory_start + (scratch_arena_capacity)};

    return memory;
}

/* init
 * Called on startup to allocate memory related to the shells lifetime.
 * Returns: exit result, EXIT_SUCCESS or EXIT_FAILURE
 */
[[nodiscard]]
char* init(Shell* rst shell)
{
    char* memory = arena_init(shell);
    if (!memory) {
        puts(RED "ncsh: could not start up, not enough memory available." RESET);
        return NULL;
    }

    if (config_init(&shell->config, &shell->arena, shell->scratch_arena) != E_SUCCESS) {
        return NULL;
    }

    if (ncreadline_init(&shell->config, &shell->input, &shell->arena) != EXIT_SUCCESS) {
        return NULL;
    }

    enum z_Result z_result = z_init(&shell->config.config_location, &shell->z_db, &shell->arena);
    if (z_result != Z_SUCCESS) {
        return NULL;
    }

    interpreter_init(shell);

    signal_init();
    if (signal_forward(SIGINT) || signal_forward(SIGWINCH)) {
        perror("ncsh: Error setting up signal handlers");
        return NULL;
    }

    return memory;
}

void cleanup(char* rst shell_memory, Shell* rst shell)
{
    // don't bother cleaning up if no shell memory allocated
    if (!shell_memory) {
        return;
    }
    if (shell->input.buffer) {
        ncreadline_exit(&shell->input);
    }
    if (shell->z_db.database_file) {
        z_exit(&shell->z_db);
    }
    free(shell_memory);
}

/* main
 * Handles initialization, exit, and the main loop of the shell.
 * Handles whether running interactively or noninteractive.
 *
 * Interactive mode several lifetimes.
 * 1. The lifetime of the shell, managed through the arena. See init and exit.
 * 2. The lifetime of the main loop of the shell, managed through the scratch arena.
 * 3. readline has its own inner lifetime via the scratch arena.
 */
[[nodiscard]]
int main(int argc, char** argv)
{
    if (argc > 1 || !isatty(STDIN_FILENO)) {
        return (int)noninteractive(argc, argv);
    }

#ifdef NCSH_START_TIME
    clock_t start = clock();
#endif

#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
    constexpr size_t clear_screen_len = sizeof(CLEAR_SCREEN MOVE_CURSOR_HOME) - 1;
    if (write(STDOUT_FILENO, CLEAR_SCREEN MOVE_CURSOR_HOME, clear_screen_len) == -1) {
        return EXIT_FAILURE;
    }
#endif
    puts(NCSH " version: " NCSH_VERSION);

    Shell shell = {0};

    char* memory = init(&shell);
    if (!memory) {
        return EXIT_FAILURE;
    }

    int exit_code = EXIT_SUCCESS;

    if (setjmp(env)) {
        goto exit;
    }

#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("ncsh startup time: %.2f milliseconds\n", elapsed_ms);
#endif

    while (1) {
        int input_result = ncreadline(&shell.input, &shell.scratch_arena);
        switch (input_result) {
        case EXIT_FAILURE: {
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        case EXIT_SUCCESS_END: {
            goto exit;
        }
        case EXIT_SUCCESS: {
            goto reset;
        }
        }

        int command_result = interpreter_run(&shell, shell.scratch_arena);
        if (command_result == EXIT_FAILURE) {
            exit_code = EXIT_FAILURE;
            break;
        }
        else if (command_result == EXIT_SUCCESS_END) {
            break;
        }

        history_add(shell.input.buffer, shell.input.pos, &shell.input.history, &shell.arena);
        ac_add(shell.input.buffer, shell.input.pos, shell.input.autocompletions_tree, &shell.arena);

    reset:
        memset(shell.input.buffer, '\0', shell.input.max_pos);
        shell.input.pos = 0;
        shell.input.max_pos = 0;
    }
exit:
    cleanup(memory, &shell);
    ncsh_write_literal("exit\n");

    return (int)exit_code;
}
