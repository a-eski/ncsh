/* Copyright ncsh by Alex Eski 2024 */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "arena.h"
#include "conf.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "interpreter/interpreter.h"
#include "noninteractive.h"
#include "io/ac.h"
#include "io/io.h"
#include "signals.h"
#include "ttyio/ttyio.h"
#include "env.h"

/* Global Variables
 * Globals should be minimized as much as possible.
 * These are only used to simplify signal handling and failure cases.
 */
jmp_buf env_jmp_buf;
sig_atomic_t vm_child_pid;
volatile int sigwinch_caught;

/* arena_init
 * Initialize arenas used for the lifteim of the shell.
 * A permanent arena and a scratch arena are allocated.
 * Returns: pointer to start of the memory block allocated.
 */
[[nodiscard]]
char* arena_init(Shell* restrict shell)
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
char* init(Shell* restrict shell, char** restrict envp)
{
    char* memory = arena_init(shell);
    if (!memory) {
        tty_color_set(TTYIO_RED_ERROR);
        tty_puts("ncsh: could not start up, not enough memory available.");
        tty_color_reset();
        return NULL;
    }

    env_new(shell, envp, &shell->arena);

    if (config_init(shell, shell->scratch_arena) != E_SUCCESS) {
        return NULL;
    }

    if (io_init(&shell->config, shell->env, &shell->input, &shell->arena) != EXIT_SUCCESS) {
        return NULL;
    }

    enum z_Result z_result = z_init(&shell->config.config_location, &shell->z_db, &shell->arena);
    if (z_result != Z_SUCCESS) {
        return NULL;
    }

    signal_init();
    if (signal_forward(SIGINT) || signal_forward(SIGWINCH)) {
        tty_perror("ncsh: Error setting up signal handlers");
        return NULL;
    }

    return memory;
}

void cleanup(char* restrict shell_memory, Shell* restrict shell)
{
    // don't bother cleaning up if no shell memory allocated
    if (!shell_memory) {
        return;
    }
    if (shell->input.buffer) {
        io_deinit(&shell->input, shell->scratch_arena);
    }
    if (shell->z_db.database_file) {
        z_exit(&shell->z_db);
    }
    free(shell_memory);
}

clock_t start;
void welcome()
{
#ifdef NCSH_START_TIME
    start = clock();
#endif

#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);
#endif

    tty_puts(NCSH " version: " NCSH_VERSION);
}

void startup_time()
{
#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    tty_println("ncsh startup time: %.2f milliseconds", elapsed_ms);
#endif
}

/* main
 * Handles initialization, exit, and the main loop of the shell.
 * Handles whether running interactively or noninteractive.
 *
 * Interactive mode several lifetimes. Examples:
 * 1. The lifetime of the shell, managed through the arena. See init and exit.
 * 2. The lifetime of the main loop of the shell, managed through the scratch arena.
 * 3. io has its own inner lifetime via the scratch arena.
 */
[[nodiscard]]
int main(int argc, char** argv, char** envp)
{

    int rv = EXIT_SUCCESS;
    if (argc > 1 || !isatty(STDIN_FILENO)) {
        rv = noninteractive(argc, argv, envp);
        return rv;
    }

    tty_init_caps();
    welcome();

    Shell shell = {0};
    char* memory = init(&shell, envp);
    if (!memory) {
        return EXIT_FAILURE;
    }

    if (setjmp(env_jmp_buf)) {
        goto exit;
    }

    rv = EXIT_SUCCESS;
    startup_time();

    while (1) {
        int input_result = io_readline(&shell.input, &shell.scratch_arena);
        switch (input_result) {
        case EXIT_FAILURE: {
            rv = EXIT_FAILURE;
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
            rv = EXIT_FAILURE;
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
    tty_println("exit");
    cleanup(memory, &shell);
    tty_deinit_caps();

    return rv;
}
