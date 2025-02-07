#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh_autocompletions.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_terminal.h"
#include "ncsh_readline.h"

/* Prompt */
#ifdef NCSH_SHORT_DIRECTORY
size_t ncsh_readline_prompt_short_directory(char* cwd, char* output)
{
    uint_fast32_t i = 1;
    uint_fast32_t last_slash_pos = 0;
    uint_fast32_t second_to_last_slash = 0;

    while (cwd[i] != '\n' && cwd[i] != '\0') {
        if (cwd[i] == '/') {
            second_to_last_slash = last_slash_pos;
            last_slash_pos = i + 1;
        }
        ++i;
    }

    memcpy(output, cwd + second_to_last_slash - 1, i - second_to_last_slash + 1);
    output[i - second_to_last_slash + 1] = '\0';

    return i - second_to_last_slash + 2; // null termination included in len
}
#endif /* ifdef NCSH_SHORT_DIRECTORY */

eskilib_nodiscard int_fast32_t ncsh_readline_prompt(struct ncsh_Terminal* terminal)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

#ifdef NCSH_SHORT_DIRECTORY
    size_t dir_len = ncsh_readline_prompt_short_directory(cwd, directory);
    printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", terminal->user.value, directory);
#else
    strcpy(directory, cwd);
    size_t dir_len = strlen(directory) + 1;
    printf(ncsh_GREEN "%s" WHITE " " ncsh_CYAN "%s" WHITE_BRIGHT " \u2771 ", terminal->user.value, directory);
#endif /* ifdef NCSH_SHORT_DIRECTORY */

    terminal->prompt_len = ncsh_terminal_prompt_size(terminal->user.length, dir_len);

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);

    return EXIT_SUCCESS;
}

eskilib_nodiscard int_fast32_t ncsh_backspace(struct ncsh_Input* input)
{
    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    --input->pos;
    if (input->max_pos > 0) {
        --input->max_pos;
    }

    ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);
    input->buffer[input->max_pos] = '\0';

    while (input->buffer[input->pos] != '\0') {
        putchar(input->buffer[input->pos]);
        ++input->pos;
    }

    fflush(stdout);

    while (input->pos > input->start_pos) {
        if (!input->pos || !input->buffer[input->pos - 1]) {
            break;
        }

        ncsh_write_literal(MOVE_CURSOR_LEFT);
        --input->pos;
    }

    return EXIT_SUCCESS;
}

eskilib_nodiscard int_fast32_t ncsh_delete(struct ncsh_Input* input)
{
    ncsh_write_literal(DELETE_STRING ERASE_CURRENT_LINE);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);

    if (input->max_pos > 0) {
        --input->max_pos;
    }

    while (input->pos < input->max_pos && input->buffer[input->pos]) {
        putchar(input->buffer[input->pos]);
        ++input->pos;
    }
    fflush(stdout);

    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    while (input->pos > input->start_pos && input->pos != 0 && input->buffer[input->pos - 1]) {
        ncsh_write_literal(MOVE_CURSOR_LEFT);
        --input->pos;
    }

    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    return EXIT_SUCCESS;
}

void ncsh_autocomplete(struct ncsh_Input* input)
{
    uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(
        input->buffer, input->pos + 1, input->current_autocompletion, input->autocompletions_tree);

    if (!autocompletions_matches_count) {
        input->current_autocompletion[0] = '\0';
        return;
    }

    input->terminal.position = ncsh_terminal_position();
    /*int len = strlen(input->current_autocompletion);
    input->terminal.position = ncsh_terminal_position();
    int lines_before = input->terminal.lines.y;
    int lines_after = lines_before;
    // printf("x: %d\n", input->terminal.lines.x + (int)input->pos + len - 1);
    if (input->terminal.lines.x + (int)input->pos + len - 1 > input->terminal.size.x) {
	++lines_after;
    }*/
    // ncsh_terminal_line_size(input->pos, &input->terminal);

    // if (lines_before != lines_after) {
        /*if (write(STDOUT_FILENO, WHITE_DIM, sizeof(WHITE_DIM) - 1) == -1)
            return;

        for (int_fast32_t i = 0; i < input->terminal.size.x - input->terminal.position.x; ++i) {
            putchar(input->current_autocompletion[i]);
        }
        putchar('\n');
        for (int_fast32_t i = input->terminal.size.x - input->terminal.position.x; i < length; ++i) {
            putchar(input->current_autocompletion[i]);
        }

        if (write(STDOUT_FILENO, RESET, sizeof(RESET) - 1) == -1)
            return;
        fflush(stdout);*/
        // return;
    // }

    printf(WHITE_DIM "%s" RESET, input->current_autocompletion);
    ncsh_terminal_move(input->terminal.position.x, input->terminal.position.y);
    fflush(stdout);
}

char ncsh_read(void)
{
    char character = 0;
    if (!read(STDIN_FILENO, &character, 1)) {
        perror(RED NCSH_ERROR_STDIN RESET);
        return EXIT_IO_FAILURE;
    }

    switch (character) {
    case ESCAPE_CHARACTER: {
        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED NCSH_ERROR_STDIN RESET);
            return EXIT_IO_FAILURE;
        }

        if (character == '[') {
            if (read(STDIN_FILENO, &character, 1) == -1) {
                perror(RED NCSH_ERROR_STDIN RESET);
                return EXIT_IO_FAILURE;
            }

            return character;
        }

        break;
    }
    case '\n':
    case '\r': {
        return character;
    }
    }

    return '\0';
}

eskilib_nodiscard int_fast32_t ncsh_tab_autocomplete(struct ncsh_Input* input,
                                                     struct ncsh_Autocompletion_Node* autocompletions_tree)
{
    ncsh_write_literal(ERASE_CURRENT_LINE "\n");

    struct ncsh_Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    int autocompletions_matches_count =
        ncsh_autocompletions_get(input->buffer, input->pos + 1, autocompletion_matches, autocompletions_tree);

    if (!autocompletions_matches_count) {
        return EXIT_SUCCESS;
    }

    if (input->buffer) {
        for (int i = 0; i < autocompletions_matches_count; ++i) {
            printf("%s%s\n", input->buffer, autocompletion_matches[i].value);
        }
    }
    else {
        for (int i = 0; i < autocompletions_matches_count; ++i) {
            printf("%s\n", autocompletion_matches[i].value);
        }
    }

    ncsh_terminal_move_up(autocompletions_matches_count);
    fflush(stdout);

    int position = 0;
    char character;

    int_fast32_t exit = EXIT_SUCCESS;
    bool continue_input = true;
    while (continue_input) {
        if ((character = ncsh_read()) == EXIT_IO_FAILURE) {
            return EXIT_FAILURE;
        }
        switch (character) {
        case UP_ARROW: {
            if (!position) {
                break;
            }
            ncsh_write_literal(MOVE_CURSOR_UP);
            --position;
            break;
        }
        case DOWN_ARROW: {
            if (position == autocompletions_matches_count - 1) {
                break;
            }
            ncsh_write_literal(MOVE_CURSOR_DOWN);
            ++position;
            break;
        }
        case '\n':
        case '\r': {
            continue_input = false;
            size_t length = strlen(autocompletion_matches[position].value) + 1;
            memcpy(input->buffer + input->pos, autocompletion_matches[position].value, length);
            input->pos += length;
            exit = EXIT_SUCCESS_EXECUTE;
            break;
        }
        default: {
            continue_input = false;
            break;
        }
        }
    }

    for (int i = 0; i < autocompletions_matches_count; ++i) {
        free(autocompletion_matches[i].value);
    }

    ncsh_terminal_move_down(autocompletions_matches_count + 1 - position);
    if (input->buffer && exit == EXIT_SUCCESS_EXECUTE) {
        printf(ncsh_YELLOW "%s\n" RESET, input->buffer);
    }
    fflush(stdout);

    return exit;
}

int_fast32_t ncsh_readline_init(struct ncsh_Config* config, struct ncsh_Input *input)
{
    input->buffer = calloc(NCSH_MAX_INPUT, sizeof(char));
    if (!input->buffer) {
        return EXIT_FAILURE;
    }

    if (ncsh_terminal_init(&input->terminal) != E_SUCCESS) {
        return EXIT_FAILURE;
    }

    enum eskilib_Result result;
    if ((result = ncsh_history_init(config->config_location, &input->history)) != E_SUCCESS) {
        free(input->buffer);
        if (result != E_FAILURE_MALLOC) {
            ncsh_history_exit(&input->history);
        }
        return EXIT_FAILURE;
    }

    input->current_autocompletion = calloc(NCSH_MAX_INPUT, sizeof(char));
    if (!input->current_autocompletion) {
        perror(RED "ncsh: Error when allocating data for autocompletion results" RESET);
        fflush(stderr);
        free(input->buffer);
        ncsh_history_exit(&input->history);
    }

    input->autocompletions_tree = ncsh_autocompletions_malloc();
    if (!input->autocompletions_tree) {
        perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
        fflush(stderr);
        free(input->buffer);
        ncsh_history_exit(&input->history);
        ncsh_autocompletions_free(input->autocompletions_tree);
        return EXIT_FAILURE;
    }
    ncsh_autocompletions_add_multiple(input->history.entries, input->history.count, input->autocompletions_tree);

    return EXIT_SUCCESS;
}

eskilib_nodiscard int_fast32_t ncsh_readline(struct ncsh_Input* input)
{
    char character;
    char temp_character;
    int exit = EXIT_SUCCESS;

    ncsh_terminal_os_init();

    while (1) {
        if (!input->pos && input->terminal.reprint_prompt == true) {
            if (ncsh_readline_prompt(&input->terminal) == EXIT_FAILURE) {
                exit = EXIT_FAILURE;
		break;
            }
            input->history_position = 0;
        }
        else {
            input->terminal.reprint_prompt = true;
        }

        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            exit = EXIT_FAILURE;
	    break;
        }

        if (character == CTRL_D) {
            ncsh_write_literal(ERASE_CURRENT_LINE);
            putchar('\n');
            fflush(stdout);
            exit = EXIT_SUCCESS_END;
	    break;
        }
        else if (character == CTRL_W) { // delete last word
            input->terminal.reprint_prompt = false;
            if (!input->pos && !input->max_pos) {
                continue;
            }

            ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);
            input->buffer[input->max_pos] = '\0';
            --input->max_pos;

            while (input->max_pos > 0 && input->buffer[input->max_pos] != ' ') {
                ncsh_write_literal(BACKSPACE_STRING);
                input->buffer[input->max_pos] = '\0';
                --input->max_pos;
            }
            fflush(stdout);

            input->pos = input->max_pos;
        }
        else if (character == CTRL_U) { // delete entire line
            input->terminal.reprint_prompt = false;
            if (!input->pos && !input->max_pos) {
                continue;
            }

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
        }
        else if (character == TAB_KEY) {
            int_fast32_t tab_autocomplete_result = ncsh_tab_autocomplete(input, input->autocompletions_tree);
	    if (tab_autocomplete_result != EXIT_SUCCESS) {
		exit = tab_autocomplete_result;
		break;
	    }

	    input->terminal.reprint_prompt = true;
            input->buffer[0] = '\0';
            input->pos = 0;
            input->max_pos = 0;

            continue;
        }
        else if (character == BACKSPACE_KEY) {
            input->terminal.reprint_prompt = false;
            if (!input->pos) {
                continue;
            }
            input->current_autocompletion[0] = '\0';
            if (ncsh_backspace(input) == EXIT_FAILURE) {
                exit = EXIT_FAILURE;
		break;
            }
        }
        else if (character == ESCAPE_CHARACTER) {
            if (read(STDIN_FILENO, &character, 1) == -1) {
                perror(RED NCSH_ERROR_STDIN RESET);
                fflush(stderr);
                exit = EXIT_FAILURE;
		break;
            }

            if (character == '[') {
                if (read(STDIN_FILENO, &character, 1) == -1) {
                    perror(RED NCSH_ERROR_STDIN RESET);
                    fflush(stderr);
                    exit = EXIT_FAILURE;
		    break;
                }

		int key_result = EXIT_SUCCESS;
                switch (character) {
                case RIGHT_ARROW: {
                    input->terminal.reprint_prompt = false;
                    if (!input->pos && !input->max_pos) {
                        continue;
                    }

                    if (input->pos == input->max_pos && input->buffer[0]) {
                        printf("%s", input->current_autocompletion);
                        for (uint_fast32_t i = 0; input->current_autocompletion[i] != '\0'; i++) {
                            input->buffer[input->pos] = input->current_autocompletion[i];
                            ++input->pos;
                        }
                        input->buffer[input->pos] = '\0';

                        if (input->pos > input->max_pos) {
                            input->max_pos = input->pos;
                        }

                        fflush(stdout);
                        input->current_autocompletion[0] = '\0';
                        break;
                    }

                    if (input->pos == NCSH_MAX_INPUT - 1 ||
                        (!input->buffer[input->pos] && !input->buffer[input->pos + 1])) {
                        continue;
                    }

                    ncsh_write_literal(MOVE_CURSOR_RIGHT);

                    ++input->pos;
                    if (input->pos > input->max_pos) {
                        input->max_pos = input->pos;
                    }

                    break;
                }
                case LEFT_ARROW: {
                    input->terminal.reprint_prompt = false;

                    if (!input->pos || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
                        continue;
                    }

                    ncsh_write_literal(MOVE_CURSOR_LEFT);

                    --input->pos;

                    break;
                }
                case UP_ARROW: {
                    input->terminal.reprint_prompt = false;

                    input->history_entry = ncsh_history_get(input->history_position, &input->history);
                    if (input->history_entry.length > 0) {
                        ++input->history_position;
                        ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

                        input->pos = input->history_entry.length - 1;
                        input->max_pos = input->history_entry.length - 1;
                        memcpy(input->buffer, input->history_entry.value, input->pos);

                        ncsh_write(input->buffer, input->pos);
                    }

                    continue;
                }
                case DOWN_ARROW: {
                    input->terminal.reprint_prompt = false;

                    if (!input->history_position) {
                        continue;
                    }

                    input->history_entry = ncsh_history_get(input->history_position - 2, &input->history);

                    ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

                    if (input->history_entry.length > 0) {
                        --input->history_position;
                        input->pos = input->history_entry.length - 1;
                        input->max_pos = input->history_entry.length - 1;
                        memcpy(input->buffer, input->history_entry.value, input->pos);

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
                        key_result = EXIT_FAILURE;
			break;
                    }

                    if (character != DELETE_KEY) {
                        continue;
                    }

                    input->terminal.reprint_prompt = false;

                    if (ncsh_delete(input) == EXIT_FAILURE) {
                        key_result = EXIT_FAILURE;
			break;
                    }

                    break;
                }
                case HOME_KEY: {
                    input->terminal.reprint_prompt = false;
                    if (!input->pos) {
                        continue;
                    }

                    ncsh_write_literal(RESTORE_CURSOR_POSITION);
                    input->pos = 0;

                    break;
                }
                case END_KEY: {
                    input->terminal.reprint_prompt = false;
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

		if (key_result != EXIT_SUCCESS) {
		    exit = key_result;
		    break;
		}
            }
            else {
                input->terminal.reprint_prompt = false;
            }
        }
        else if (character == '\n' || character == '\r') {
            if (!input->pos && !input->buffer[input->pos]) {
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
	    ncsh_terminal_os_reset();
            exit = EXIT_SUCCESS_EXECUTE;
	    break;
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

                if (!input->pos) {
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

                if (input->pos > input->max_pos) {
                    input->max_pos = input->pos;
                }

                if (input->pos == input->max_pos) {
                    input->buffer[input->pos] = '\0';
                }

                fflush(stdout);

                if (!input->pos || input->buffer[1] == '\0') {
                    continue;
                }

                while (input->pos > input->start_pos + 1) {
                    ncsh_write_literal(MOVE_CURSOR_LEFT);
                    --input->pos;
                }
            }
            else { // end of line insertions
	        /*assert(input->pos <= INT_MAX);
		if ((input->terminal.lines.x + (int)input->pos / input->terminal.lines.y) > input->terminal.size.x) {
		    ++input->terminal.lines.y;
		    // input->terminal.lines.x = input->terminal.lines.x + (int)input->pos - input->terminal.size.x;
		    putchar('\n');
		}*/
                putchar(character);
                fflush(stdout);
                input->buffer[input->pos++] = character;

                if (input->pos > input->max_pos) {
                    input->max_pos = input->pos;
                }

                if (input->pos == input->max_pos) {
                    input->buffer[input->pos] = '\0';
                }

                ncsh_write_literal(ERASE_CURRENT_LINE);
            }
        }

        // autocompletion logic
        if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0') {
            continue;
        }
        else if (input->buffer[0] < 32) { // exclude control characters from autocomplete
            memset(input->buffer, '\0', input->max_pos);
            input->pos = 0;
            input->max_pos = 0;
	    input->terminal.reprint_prompt = false;
            continue;
	}

        ncsh_autocomplete(input);
    }


    ncsh_terminal_os_reset();
    return exit;
}

void ncsh_readline_exit(struct ncsh_Input* input)
{
    free(input->buffer);
    ncsh_history_exit(&input->history);
    free(input->current_autocompletion);
    ncsh_autocompletions_free(input->autocompletions_tree);
}
