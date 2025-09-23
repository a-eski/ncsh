/* Copyright ncsh by Alex Eski 2024 */

#ifndef _POXIC_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif /* ifndef _POXIC_C_SOURCE */

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "eskilib/str.h"
#include "debug.h"
#include "io/bestline.h"
#include "io/bestout.h"
#include "arena.h"
#include "conf.h"
#include "defines.h"
#include "eskilib/eresult.h"
#include "interpreter/interpreter.h"
#include "io/ac.h"
#include "io/prompt.h"
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

Input* input_;
Arena* scratch_;
Arena* arena_;

/* arena_init
 * Initialize arenas used for the lifteim of the shell.
 * A permanent arena and a scratch arena are allocated.
 * Returns: pointer to start of the memory block allocated.
 */
[[nodiscard]]
static char* arena_init(Shell* restrict shell)
{
    constexpr int arena_capacity = 1 << 24;
    constexpr int scratch_capacity = 1 << 20;
    constexpr int total_capacity = arena_capacity + scratch_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        return NULL;
    }

    shell->arena = (Arena){.start = memory, .end = memory + (arena_capacity)};
    char* scratch_memory_start = memory + (arena_capacity + 1);
    shell->scratch =
        (Arena){.start = scratch_memory_start, .end = scratch_memory_start + (scratch_capacity)};

    return memory;
}

/* arena_noninteractive_init
 * Initialize arenas used for the lifteim of the shell.
 * A permanent arena and a scratch arena are allocated.
 * Returns: pointer to start of the memory block allocated.
 */
[[nodiscard]]
static char* arena_noninteractive_init(Shell* restrict shell)
{
    constexpr int arena_capacity = 1 << 20;
    constexpr int scratch_capacity = 1 << 16;
    constexpr int total_capacity = arena_capacity + scratch_capacity;

    char* memory = malloc(total_capacity);
    if (!memory) {
        return NULL;
    }

    shell->arena = (Arena){.start = memory, .end = memory + (arena_capacity)};
    char* scratch_memory_start = memory + (arena_capacity + 1);
    shell->scratch =
        (Arena){.start = scratch_memory_start, .end = scratch_memory_start + (scratch_capacity)};

    return memory;
}

void completion(const char *buf, int pos, bestlineCompletions *lc) {
    if (pos <= 0 || !buf) {
        return;
    }

    uint8_t ac_matches_count =
        ac_first((char*)buf, input_->current_autocompletion, input_->autocompletions_tree, *input_->scratch);

    if (!ac_matches_count) {
        if (input_->current_autocompletion[0] == '\0') {
            return;
        }
        input_->current_autocompletion[0] = '\0';
        input_->current_autocompletion_len = 0;
        return;
    }

    if (input_->current_autocompletion) {
        Str* completion = estrcat(&Str((char*)buf, (size_t)(pos + 1)), &Str_Get(input_->current_autocompletion), input_->scratch);
        bestlineAddCompletion(lc, completion->value);
    }
}

char *hints(const char *buf, const char **ansi1, const char **ansi2) {
    if (!buf || !*buf) {
        return NULL;
    }

    uint8_t ac_matches_count =
        ac_first((char*)buf, input_->current_autocompletion, input_->autocompletions_tree, *input_->scratch);

    if (!ac_matches_count) {
        if (input_->current_autocompletion[0] == '\0') {
            return NULL;
        }
        input_->current_autocompletion[0] = '\0';
        input_->current_autocompletion_len = 0;
        return NULL;
    }

    if (input_->current_autocompletion) {
        // "\033[2m"
        *ansi1 = "\033[35m"; /* magenta foreground */
        *ansi2 = "\033[39m"; /* reset foreground */
        return input_->current_autocompletion;
    }
    return NULL;
}

/* hooks into bestline history loading to populate trie used for autocompletions */
void ac_add_when_history_expanded(const char *in, int n) {
    assert(n > 0);
    ac_add((char *)in, (size_t)n, input_->autocompletions_tree, arena_);
}

/* init
 * Called on startup to allocate memory related to the shells lifetime.
 * Returns: exit result, EXIT_SUCCESS or EXIT_FAILURE
 */
[[nodiscard]]
static char* init(Shell* restrict shell, char** restrict envp)
{
    char* memory = arena_init(shell);
    if (!memory) {
        tty_color_set(TTYIO_RED_ERROR);
        bestlineWriteStr(STDERR_FILENO, Str_Lit("ncsh: could not start up, not enough memory available.\n"));
        tty_color_reset();
        return NULL;
    }

    env_new(shell, envp, &shell->arena);

    if (conf_init(shell) != E_SUCCESS) {
        return NULL;
    }

    prompt_init();
    Str user_key = Str_Lit(NCSH_USER_VAL);
    shell->input.user = env_add_or_get(shell->env, user_key);
    if (!shell->input.user->value) {
        shell->input.user->length = 0;
    }

    shell->input.current_autocompletion = arena_malloc(&shell->arena, NCSH_MAX_INPUT, char);
    shell->input.autocompletions_tree = ac_alloc(&shell->arena);

    enum z_Result z_result = z_init(&shell->config.location, &shell->z_db, &shell->arena);
    if (z_result != Z_SUCCESS) {
        return NULL;
    }

    if ((shell->pgid = signal_init()) < 0) {
        bestlineWriteStr(STDERR_FILENO, Str_Lit("ncsh: fatal error while initializing signal handlers\n"));
        return NULL;
    }

    input_ = &shell->input;
    input_->scratch = &shell->scratch;
    arena_ = &shell->arena;
    bestlineSetHintsCallback(hints);
    bestlineSetCompletionCallback(completion);
    bestlineSetOnHistoryLoadedCallback(ac_add_when_history_expanded);
    bestlineHistoryLoad(shell->config.history_file.value);
    shell->arena = *arena_;

    return memory;
}

static void cleanup(char* restrict shell_memory, Shell* restrict shell)
{
    // don't bother cleaning up if no shell memory allocated
    if (!shell_memory) {
        return;
    }
    if (shell->config.history_file.value) {
        bestlineHistorySave(shell->config.history_file.value);
    }
    if (shell->input.buffer) {
        free(shell->input.buffer);
    }
    if (shell->z_db.database_file) {
        z_exit(&shell->z_db);
    }
    free(shell_memory);
}

static clock_t start;
static void welcome()
{
#ifdef NCSH_START_TIME
    start = clock();
#endif

#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
    tty_send(&tcaps.scr_clr);
    tty_send(&tcaps.cursor_home);
#endif

    bestlineWriteStr(STDOUT_FILENO, Str_Lit(NCSH " version: " NCSH_VERSION "\n"));
}

static void startup_time()
{
#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    tty_println("ncsh startup time: %.2f milliseconds", elapsed_ms);
#endif
}

/* noninteractive
 * Main noninteractive loop of the shell.
 * Runs when calling shell via command-line like /bin/ncsh ls or via scripts.
 * Much simpler than interactive loop, parses input from command-line and sends it to the VM.
 * Returns: exit status, see defines.h (EXIT_...)
 */
[[nodiscard]]
static int noninteractive(int argc, char** restrict argv, char** restrict envp)
{
    assert(argc > 1); // 1 because first arg is ncsh
    assert(argv);
    assert(strstr(argv[0], "ncsh"));

    // noninteractive does not support no args
    if (argc <= 1) {
        return EXIT_FAILURE;
    }

    debug("ncsh running in noninteractive mode.");
    debug_argsv(argc, argv);

    tty_init_caps();

    Shell shell = {0};
    char* memory = arena_noninteractive_init(&shell);
    if (!memory) {
        tty_color_set(TTYIO_RED_ERROR);
        bestlineWriteStr(STDERR_FILENO, Str_Lit("ncsh: could not start up, not enough memory available.\n"));
        tty_color_reset();
        tty_deinit_caps();
        return EXIT_FAILURE;
    }

    env_new(&shell, envp, &shell.arena);

    int rv = EXIT_SUCCESS;
    if (conf_init(&shell) != E_SUCCESS) {
        rv = EXIT_FAILURE;
        goto exit;
    }

    rv = interpreter_run_noninteractive(argv + 1, (size_t)argc - 1, &shell);

exit:
    tty_deinit_caps();
    free(memory);
    return rv;
}

/* main
 * Handles initialization, exit, and the main loop of the shell.
 * Handles whether running interactively or noninteractive.
 *
 * Interactive mode several lifetimes. Examples:
 * 1. The lifetime of the shell, managed through the arena. See init and exit.
 * 2. The lifetime of the main loop of the shell, managed through the scratch arena.
 * 3. different modules like the interpreter have their own lifetime via the scratch arena.
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

    startup_time();

    Str prompt;
    while ((prompt = prompt_get(&shell.input, &shell.scratch)).value &&
        (shell.input.buffer = bestline(prompt.value))) {
        shell.input.pos = strlen(shell.input.buffer) + 1;

        int command_result = interpreter_run(&shell, shell.scratch);
        if (command_result == EXIT_FAILURE) {
            rv = EXIT_FAILURE;
            break;
        }
        else if (command_result == EXIT_SUCCESS_END) {
            break;
        }

        bestlineHistoryAdd(shell.input.buffer);
        ac_add(shell.input.buffer, shell.input.pos, shell.input.autocompletions_tree, &shell.arena);
        shell.input.pos = 0;
        free(shell.input.buffer);
    }

exit:
    tty_puts("exit");
    cleanup(memory, &shell);
    tty_deinit_caps();
    return rv;
}
