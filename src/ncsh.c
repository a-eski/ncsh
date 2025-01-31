// Copyright (c) ncsh by Alex Eski 2024

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
#include "eskilib/eskilib_defines.h"
#include "eskilib/eskilib_result.h"
#include "eskilib/eskilib_string.h"
#include "ncsh.h"
#include "ncsh_autocompletions.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_readline.h"
#include "ncsh_terminal.h"
#include "ncsh_types.h"
#include "ncsh_vm.h"

void ncsh_exit(struct ncsh_Shell* shell)
{
    ncsh_terminal_free(&shell->terminal);
    free(shell->input.buffer);

    ncsh_config_free(&shell->config);

    ncsh_parser_args_free(&shell->args);

    ncsh_history_exit(&shell->history);

    free(shell->current_autocompletion);
    ncsh_autocompletions_free(shell->autocompletions_tree);

    z_exit(&shell->z_db);

    ncsh_terminal_os_reset();
}

eskilib_nodiscard int_fast32_t ncsh_init(struct ncsh_Shell* shell)
{
    if (ncsh_terminal_malloc(&shell->terminal) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    shell->input.buffer = calloc(NCSH_MAX_INPUT, sizeof(char));
    if (!shell->input.buffer) {
        free(shell->terminal.directory.value);
        return EXIT_FAILURE;
    }

    enum eskilib_Result result;
    if ((result = ncsh_config_init(&shell->config)) != E_SUCCESS) {
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        if (result != E_FAILURE_MALLOC) {
            ncsh_config_free(&shell->config);
        }
        return EXIT_FAILURE;
    }

    if ((result = ncsh_parser_args_malloc(&shell->args)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        ncsh_config_free(&shell->config);
        if (result != E_FAILURE_MALLOC) {
            ncsh_parser_args_free(&shell->args);
        }
        return EXIT_FAILURE;
    }

    if ((result = ncsh_history_init(shell->config.config_location, &shell->history)) != E_SUCCESS) {
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        if (result != E_FAILURE_MALLOC) {
            ncsh_history_exit(&shell->history);
        }
        return EXIT_FAILURE;
    }

    shell->current_autocompletion = calloc(NCSH_MAX_INPUT, sizeof(char));
    if (!shell->current_autocompletion) {
        perror(RED "ncsh: Error when allocating data for autocompletion results" RESET);
        fflush(stderr);
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
    }

    shell->autocompletions_tree = ncsh_autocompletions_malloc();
    if (!shell->autocompletions_tree) {
        perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
        fflush(stderr);
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
        ncsh_autocompletions_free(shell->autocompletions_tree);
        return EXIT_FAILURE;
    }
    ncsh_autocompletions_add_multiple(shell->history.entries, shell->history.count, shell->autocompletions_tree);

    enum z_Result z_result = z_init(shell->config.config_location, &shell->z_db);
    if (z_result != Z_SUCCESS) {
        ncsh_terminal_free(&shell->terminal);
        free(shell->input.buffer);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
        free(shell->current_autocompletion);
        ncsh_autocompletions_free(shell->autocompletions_tree);
        if (z_result != Z_MALLOC_ERROR) {
            z_exit(&shell->z_db);
        }
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int_fast32_t ncsh(void)
{
#ifdef NCSH_START_TIME
    clock_t start = clock();
#endif

#ifdef NCSH_CLEAR_SCREEN_ON_STARTUP
    if (write(STDOUT_FILENO, CLEAR_SCREEN MOVE_CURSOR_HOME, sizeof(CLEAR_SCREEN MOVE_CURSOR_HOME) - 1) == -1) {
        return EXIT_FAILURE;
    }
#endif

    int_fast32_t exit_code = EXIT_SUCCESS;
    int_fast32_t input_result = 0;
    int_fast32_t command_result = 0;

    struct ncsh_Shell shell = {0};
    if (ncsh_init(&shell) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

#ifdef NCSH_START_TIME
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    printf("ncsh: startup time: %.2f milliseconds\n", elapsed_ms);
#endif

    while (1) {
        input_result = ncsh_readline(&shell.input, &shell);
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

        ncsh_parser_parse(shell.input.buffer, shell.input.pos, &shell.args);

        command_result = ncsh_vm_execute(&shell);
        switch (command_result) {
        case NCSH_COMMAND_EXIT_FAILURE: {
            exit_code = EXIT_FAILURE;
            goto exit;
        }
        case NCSH_COMMAND_EXIT: {
            goto exit;
        }
        case NCSH_COMMAND_SYNTAX_ERROR:
        case NCSH_COMMAND_FAILED_CONTINUE: {
            goto reset;
        }
        }

        ncsh_history_add(shell.input.buffer, shell.input.pos, &shell.history);
        ncsh_autocompletions_add(shell.input.buffer, shell.input.pos, shell.autocompletions_tree);

    reset:
        ncsh_parser_args_free_values(&shell.args);
        memset(shell.input.buffer, '\0', shell.input.max_pos);
        shell.input.pos = 0;
        shell.input.max_pos = 0;
        shell.args.count = 0;
        shell.args.values[0] = NULL;
    }

exit:
    ncsh_exit(&shell);

    return exit_code;
}
