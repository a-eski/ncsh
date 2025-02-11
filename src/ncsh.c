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
#include "ncsh.h"
#include "ncsh_autocompletions.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_history.h"
#include "ncsh_parser.h"
#include "ncsh_readline.h"
#include "ncsh_types.h"
#include "ncsh_vm.h"

void ncsh_exit(struct ncsh_Shell* shell)
{
    ncsh_config_free(&shell->config);

    ncsh_readline_exit(&shell->input);

    ncsh_parser_args_free(&shell->args);

    z_exit(&shell->z_db);
}

eskilib_nodiscard int_fast32_t ncsh_init(struct ncsh_Shell* shell)
{
    enum eskilib_Result result;
    if ((result = ncsh_config_init(&shell->config)) != E_SUCCESS) {
        if (result != E_FAILURE_MALLOC) {
            ncsh_config_free(&shell->config);
        }
        return EXIT_FAILURE;
    }

    if (ncsh_readline_init(&shell->config, &shell->input) != EXIT_SUCCESS) {
        ncsh_config_free(&shell->config);
        ncsh_readline_exit(&shell->input);
    }

    if ((result = ncsh_parser_args_malloc(&shell->args)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        ncsh_config_free(&shell->config);
        ncsh_readline_exit(&shell->input);
        if (result != E_FAILURE_MALLOC) {
            ncsh_parser_args_free(&shell->args);
        }
        return EXIT_FAILURE;
    }

    enum z_Result z_result = z_init(shell->config.config_location, &shell->z_db);
    if (z_result != Z_SUCCESS) {
        ncsh_config_free(&shell->config);
        ncsh_readline_exit(&shell->input);
        ncsh_parser_args_free(&shell->args);
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
        input_result = ncsh_readline(&shell.input);
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

        ncsh_history_add(shell.input.buffer, shell.input.pos, &shell.input.history);
        ncsh_autocompletions_add(shell.input.buffer, shell.input.pos, shell.input.autocompletions_tree);

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
