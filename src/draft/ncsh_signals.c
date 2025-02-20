// Copyright (c) ncsh by Alex Eski 2024

/* from VM */
/*if (setpgid(pid, pid) == 0) {
                    perror(RED "ncsh: Error setting up process group ID for child process" RESET);
                }*/


/*if (ncsh_vm_signal_forward(SIGINT) ||
                ncsh_vm_signal_forward(SIGHUP) ||
                ncsh_vm_signal_forward(SIGTERM) ||
                ncsh_vm_signal_forward(SIGQUIT) ||
                ncsh_vm_signal_forward(SIGUSR1) ||
                ncsh_vm_signal_forward(SIGUSR2)) {
                perror("ncsh: Error setting up signal handlers");
                return NCSH_COMMAND_EXIT_FAILURE;
            }*/

/* from readline */
/* Need to figure out how to handle SIGWINCH signal */

#define _POSIX_C_SOURCE 200809L
// #include <signal.h>
/* Signals */
/*static _Atomic bool ncsh_screen_size_changed = false;

static inline void ncsh_screen_size_changed_set(bool value)
{
    ncsh_screen_size_changed = value;
}

static inline bool ncsh_screen_size_changed_get()
{
    return ncsh_screen_size_changed;
}

// int signum, siginfo_t *info, void *context
// SIGWINCH handler for terminal resize
static void ncsh_sigwinch_handler(int signum)
{
    (void)signum;
    ncsh_screen_size_changed_set(true);
}

static int ncsh_screen_size_change_handler()
{
    struct sigaction act = {0};
    sigemptyset(&act.sa_mask);
    act.sa_handler = ncsh_sigwinch_handler;
    act.sa_flags = SIGWINCH;
    if (sigaction(SIGWINCH, &act, NULL))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}*/

eskilib_nodiscard int_fast32_t ncsh_init(struct ncsh* shell)
{
    shell->prompt_info.user = getenv("USER");

    enum eskilib_Result result;
    if ((result = ncsh_config_init(&shell->config)) != E_SUCCESS) {
        if (result != E_FAILURE_MALLOC)
            ncsh_config_free(&shell->config);
        return EXIT_FAILURE;
    }

    if ((result = ncsh_parser_args_malloc(&shell->args)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating memory for parser" RESET);
        fflush(stderr);
        ncsh_config_free(&shell->config);
        if (result != E_FAILURE_MALLOC)
            ncsh_parser_args_free(&shell->args);
        return EXIT_FAILURE;
    }

    if ((result = ncsh_history_init(shell->config.config_location, &shell->history)) != E_SUCCESS) {
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        if (result != E_FAILURE_MALLOC)
            ncsh_history_exit(&shell->history);
        return EXIT_FAILURE;
    }

    shell->current_autocompletion = malloc(NCSH_MAX_INPUT);
    if (!shell->current_autocompletion) {
        perror(RED "ncsh: Error when allocating data for autocompletion results" RESET);
        fflush(stderr);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
    }

    shell->autocompletions_tree = ncsh_autocompletions_malloc();
    if (!shell->autocompletions_tree) {
        perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
        fflush(stderr);
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
        ncsh_autocompletions_free(shell->autocompletions_tree);
        return EXIT_FAILURE;
    }
    ncsh_autocompletions_add_multiple(shell->history.entries, shell->history.count, shell->autocompletions_tree);

    enum z_Result z_result = z_init(shell->config.config_location, &shell->z_db);
    if (z_result != Z_SUCCESS) {
        ncsh_config_free(&shell->config);
        ncsh_parser_args_free(&shell->args);
        ncsh_history_exit(&shell->history);
        free(shell->current_autocompletion);
        ncsh_autocompletions_free(shell->autocompletions_tree);
        if (z_result != Z_MALLOC_ERROR)
            z_exit(&shell->z_db);
        return EXIT_FAILURE;
    }

    ncsh_terminal_init();
    ncsh_terminal_size_set();
    shell->terminal_size = ncsh_terminal_size_get();
    /*if (ncsh_screen_size_change_handler())
    {
        perror(RED "ncsh: error setting up SIGWINCH signal handler" RESET);
        ncsh_exit(shell);
        return EXIT_FAILURE;
    }*/

    return EXIT_SUCCESS;
}

eskilib_nodiscard int_fast32_t ncsh_readline(struct ncsh_Input* input, struct ncsh* shell)
{
    char character;
    char temp_character;

    while (1) {
        /*shell->terminal_position = ncsh_terminal_position();
        shell->terminal_size = ncsh_terminal_size();
        if (shell->terminal_position.x >= shell->terminal_size.x)
        {
            putchar('\n');
            fflush(stdout);
        }*/

        if (input->pos == 0 && shell->prompt_info.reprint_prompt == true) {
            if (ncsh_prompt(shell->prompt_info) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            shell->history_position = 0;
        }
        else {
            shell->prompt_info.reprint_prompt = true;
        }

        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            return EXIT_FAILURE;
        }

        if (character == CTRL_D) {
            putchar('\n');
            return EXIT_SUCCESS_END;
        }
        else if (character == CTRL_W) { // delete last word
            shell->prompt_info.reprint_prompt = false;
            if (input->pos == 0 && input->max_pos == 0)
                continue;

            ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);
            input->buffer[input->max_pos] = '\0';
            --input->max_pos;

            while (input->max_pos > 0 && input->buffer[input->max_pos] != ' ') {
                ncsh_write_literal(BACKSPACE_STRING);
                input->buffer[input->max_pos] = '\0';
                --input->max_pos;
            }
            fflush(stdout);

            input->pos = 0;
            shell->args.count = 0;
            shell->args.values[0] = NULL;
        }
        else if (character == CTRL_U) { // delete entire line
            shell->prompt_info.reprint_prompt = false;
            if (input->pos == 0 && input->max_pos == 0)
                continue;

            ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);
            input->buffer[input->max_pos] = '\0';
            --input->max_pos;

            while (input->max_pos > 0) {
                ncsh_write_literal(BACKSPACE_STRING);
                input->buffer[input->max_pos] = '\0';
                --input->max_pos;
            }
            fflush(stdout);

            input->pos = 0;
            shell->args.count = 0;
            shell->args.values[0] = NULL;
        }
        else if (character == TAB_KEY) {
            int_fast32_t tab_autocomplete_result = ncsh_tab_autocomplete(input, shell->autocompletions_tree);
            switch (tab_autocomplete_result) {
            case EXIT_FAILURE: {
                return EXIT_FAILURE;
            }
            case EXIT_SUCCESS_EXECUTE: {
                return EXIT_SUCCESS_EXECUTE;
            }
            case EXIT_SUCCESS: {
                shell->prompt_info.reprint_prompt = true;
                input->buffer[0] = '\0';
                input->pos = 0;
                input->max_pos = 0;
                shell->args.count = 0;
                shell->args.values[0] = NULL;
                break;
            }
            }

            continue;
        }
        else if (character == BACKSPACE_KEY) {
            shell->prompt_info.reprint_prompt = false;
            if (input->pos == 0) {
                continue;
            }
            shell->current_autocompletion[0] = '\0';
            if (ncsh_backspace(input) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
        }
        else if (character == ESCAPE_CHARACTER) {
            if (read(STDIN_FILENO, &character, 1) == -1) {
                perror(RED NCSH_ERROR_STDIN RESET);
                fflush(stderr);
                return EXIT_FAILURE;
            }

            if (character == '[') {
                if (read(STDIN_FILENO, &character, 1) == -1) {
                    perror(RED NCSH_ERROR_STDIN RESET);
                    fflush(stderr);
                    return EXIT_FAILURE;
                }

                switch (character) {
                case RIGHT_ARROW: {
                    shell->prompt_info.reprint_prompt = false;
                    if (input->pos == 0 && input->max_pos == 0)
                        continue;

                    if (input->pos == input->max_pos && input->buffer[0]) {
                        printf("%s", shell->current_autocompletion);
                        for (uint_fast32_t i = 0; shell->current_autocompletion[i] != '\0'; i++) {
                            input->buffer[input->pos] = shell->current_autocompletion[i];
                            ++input->pos;
                        }
                        input->buffer[input->pos] = '\0';
                        if (input->pos > input->max_pos)
                            input->max_pos = input->pos;

                        fflush(stdout);
                        shell->current_autocompletion[0] = '\0';
                        break;
                    }

                    if (input->pos == NCSH_MAX_INPUT - 1 ||
                        (!input->buffer[input->pos] && !input->buffer[input->pos + 1]))
                        continue;

                    ncsh_write_literal(MOVE_CURSOR_RIGHT);

                    ++input->pos;
                    if (input->pos > input->max_pos)
                        input->max_pos = input->pos;

                    break;
                }
                case LEFT_ARROW: {
                    shell->prompt_info.reprint_prompt = false;

                    if (input->pos == 0 || (!input->buffer[input->pos] && !input->buffer[input->pos - 1]))
                        continue;

                    ncsh_write_literal(MOVE_CURSOR_LEFT);

                    --input->pos;

                    break;
                }
                case UP_ARROW: {
                    shell->prompt_info.reprint_prompt = false;

                    shell->history_entry = ncsh_history_get(shell->history_position, &shell->history);
                    if (shell->history_entry.length > 0) {
                        ++shell->history_position;
                        ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

                        input->pos = shell->history_entry.length - 1;
                        input->max_pos = shell->history_entry.length - 1;
                        memcpy(input->buffer, shell->history_entry.value, input->pos);

                        ncsh_write(input->buffer, input->pos);
                    }

                    continue;
                }
                case DOWN_ARROW: {
                    shell->prompt_info.reprint_prompt = false;

                    if (shell->history_position == 0)
                        continue;

                    shell->history_entry = ncsh_history_get(shell->history_position - 2, &shell->history);

                    ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

                    if (shell->history_entry.length > 0) {
                        --shell->history_position;
                        input->pos = shell->history_entry.length - 1;
                        input->max_pos = shell->history_entry.length - 1;
                        memcpy(input->buffer, shell->history_entry.value, input->pos);

                        ncsh_write(input->buffer, input->pos);
                    }
                    else {
                        input->buffer[0] = '\0';
                        input->pos = 0;
                        input->max_pos = 0;
                    }

                    continue;
                }
                case DELETE_PREFIX_KEY: {
                    if (read(STDIN_FILENO, &character, 1) == -1) {
                        perror(RED NCSH_ERROR_STDIN RESET);
                        fflush(stderr);
                        return EXIT_FAILURE;
                    }

                    if (character != DELETE_KEY)
                        continue;

                    shell->prompt_info.reprint_prompt = false;

                    if (ncsh_delete(input) == EXIT_FAILURE)
                        return EXIT_FAILURE;

                    break;
                }
                case HOME_KEY: {
                    shell->prompt_info.reprint_prompt = false;
                    if (input->pos == 0) {
                        continue;
                    }

                    ncsh_write_literal(RESTORE_CURSOR_POSITION);
                    input->pos = 0;

                    break;
                }
                case END_KEY: {
                    shell->prompt_info.reprint_prompt = false;
                    if (input->pos == input->max_pos) {
                        continue;
                    }

                    while (input->buffer[input->pos]) {
                        ncsh_write_literal(MOVE_CURSOR_RIGHT);
                        ++input->pos;
                    }

                    break;
                }
                }
            }
        }
        else if (character == '\n' || character == '\r') {
            if (input->pos == 0 && !input->buffer[input->pos]) {
                putchar('\n');
                fflush(stdout);
                continue;
            }

            while (input->pos < input->max_pos && input->buffer[input->pos]) {
                ++input->pos;
                ncsh_write_literal(MOVE_CURSOR_RIGHT);
            }

            while (input->pos > 1 && input->buffer[input->pos - 1] == ' ') {
                --input->pos;
            }

            input->buffer[input->pos++] = '\0';

            ncsh_write_literal(ERASE_CURRENT_LINE "\n");
            fflush(stdout);

            return EXIT_SUCCESS_EXECUTE;
        }
        else {
            if (input->pos == NCSH_MAX_INPUT - 1) {
                fputs(RED "\nncsh: Hit max input.\n" RESET, stderr);
                input->buffer[0] = '\0';
                input->pos = 0;
                input->max_pos = 0;
                continue;
            }

            // midline insertions
            if (input->pos < input->max_pos && input->buffer[input->pos]) {
                input->start_pos = input->pos;

                if (input->pos == 0) {
                    temp_character = input->buffer[0];
                    input->buffer[0] = character;
                    putchar(character);
                    character = temp_character;
                    ++input->pos;
                }

                for (uint_fast32_t i = input->pos - 1; i < input->max_pos && i < NCSH_MAX_INPUT; ++i) {
                    temp_character = character;
                    character = input->buffer[i + 1];
                    input->buffer[i + 1] = temp_character;
                    putchar(temp_character);
                    ++input->pos;
                }

                if (input->pos > input->max_pos)
                    input->max_pos = input->pos;

                if (input->pos == input->max_pos)
                    input->buffer[input->pos] = '\0';

                fflush(stdout);

                if (input->pos == 0 || input->buffer[1] == '\0')
                    continue;

                while (input->pos > input->start_pos + 1) {
                    ncsh_write_literal(MOVE_CURSOR_LEFT);
                    --input->pos;
                }
            }
            else { // end of line insertions
                putchar(character);
                fflush(stdout);
                input->buffer[input->pos++] = character;

                if (input->pos > input->max_pos)
                    input->max_pos = input->pos;

                if (input->pos == input->max_pos)
                    input->buffer[input->pos] = '\0';

                ncsh_write_literal(ERASE_CURRENT_LINE);
            }
        }

        // autocompletion logic
        if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0')
            continue;

        ncsh_autocomplete(input, shell);

        /*if (ncsh_screen_size_changed_get())
        {
                ncsh_screen_size_changed_set(false);
            ncsh_terminal_size_set();
            shell->terminal_size = ncsh_terminal_size_get();
            printf("Setting screen size after SIGWINCH %d, %d\n", shell->terminal_size.x, shell->terminal_size.y);
        }*/
    }
}
