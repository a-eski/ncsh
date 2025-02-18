#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "eskilib/eskilib_result.h"
#include "ncsh_autocompletions.h"
#include "ncsh_config.h"
#include "ncsh_defines.h"
#include "ncsh_terminal.h"
#include "ncsh_readline.h"

/* Prompt */
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
size_t ncsh_readline_prompt_short_directory(char* cwd, char* output)
{
    uint_fast32_t i = 1;
    uint_fast32_t last_slash_pos = 0;
    uint_fast32_t second_to_last_slash_pos = 0;

    while (cwd[i] != '\n' && cwd[i] != '\0') {
        if (cwd[i] == '/') {
            second_to_last_slash_pos = last_slash_pos;
            last_slash_pos = i + 1;
        }
        ++i;
    }

    if (second_to_last_slash_pos != 0) { // has at least 3 slashes
    	memcpy(output, cwd + second_to_last_slash_pos - 1, i - second_to_last_slash_pos + 1);
	output[i - second_to_last_slash_pos + 1] = '\0';
    	return i - second_to_last_slash_pos + 2; // null termination included in len
    }

    if (last_slash_pos == 0) { // 1 slash at beginning of cwd
	memcpy(output, cwd, i);
	return i;
    }

    memcpy(output, cwd + last_slash_pos - 1, i - last_slash_pos + 1); // has 2 slashes
     return i - last_slash_pos + 2; // null termination included in len
}

int_fast32_t ncsh_readline_prompt_short_directory_print(struct ncsh_Input* input)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    size_t dir_len = ncsh_readline_prompt_short_directory(cwd, directory);
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value, directory);
    input->prompt_len = ncsh_terminal_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, directory);
    input->prompt_len = ncsh_terminal_prompt_size(0, dir_len);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */
    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */

#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
int_fast32_t ncsh_readline_prompt_directory_print(struct ncsh_Input* input)
{
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    size_t dir_len = strlen(cwd) + 1;

#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" WHITE_BRIGHT " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value, cwd);
    input->prompt_len = ncsh_terminal_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, cwd);
    input->prompt_len = ncsh_terminal_prompt_size(0, dir_len);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);

    return EXIT_SUCCESS;
}
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL */

#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
int_fast32_t ncsh_readline_prompt_no_directory_print(struct ncsh_Input* input) {
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value);
    input->prompt_len = ncsh_terminal_prompt_size(input->user.length, 0);
#else
    printf(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);
    input->prompt_len = ncsh_terminal_prompt_size(0, 0);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif

eskilib_nodiscard int_fast32_t ncsh_readline_prompt(struct ncsh_Input* input)
{
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    return ncsh_readline_prompt_short_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    return ncsh_readline_prompt_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    return ncsh_readline_prompt_no_directory_print(input);
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */
}

// IO
eskilib_nodiscard int_fast32_t ncsh_readline_backspace(struct ncsh_Input* input)
{
    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    input->current_autocompletion[0] = '\0';

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

eskilib_nodiscard int_fast32_t ncsh_readline_delete(struct ncsh_Input* input)
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

int ncsh_readline_line_reset(struct ncsh_Input* input)
{
    int pos = input->max_pos - input->pos;
    if (pos > 0) {
        ncsh_terminal_move_right(pos);
	ncsh_write_literal(ERASE_CURRENT_LINE);
	ncsh_terminal_move_left(pos);
	return EXIT_SUCCESS;
    }

    ncsh_write_literal(ERASE_CURRENT_LINE);
    return EXIT_SUCCESS;
}

void ncsh_readline_autocomplete(struct ncsh_Input* input)
{
    if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0') {
        return;
    }
    else if (input->buffer[0] < 32) { // exclude control characters from autocomplete
        ncsh_readline_line_reset(input);
        memset(input->buffer, '\0', input->max_pos);
        input->pos = 0;
        input->max_pos = 0;
        return;
    }

    uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(
        input->buffer, input->pos + 1, input->current_autocompletion, input->autocompletions_tree);

    if (!autocompletions_matches_count) {
        input->current_autocompletion[0] = '\0';
        ncsh_readline_line_reset(input);
        return;
    }

    struct ncsh_Coordinates previous_pos = ncsh_terminal_position();
    printf(ERASE_CURRENT_LINE WHITE_DIM "%s" RESET, input->current_autocompletion);
    ncsh_terminal_move(previous_pos.x, previous_pos.y);
    fflush(stdout);
}

char ncsh_readline_read(void)
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

eskilib_nodiscard int_fast32_t ncsh_readline_tab_autocomplete(struct ncsh_Input* input,
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
        if ((character = ncsh_readline_read()) == EXIT_IO_FAILURE) {
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
    input->max_x = ncsh_terminal_size_x();
    input->max_y = ncsh_terminal_size_x();
    input->user.value = getenv("USER");
    input->user.length = strlen(input->user.value) + 1;
    input->buffer = calloc(NCSH_MAX_INPUT, sizeof(char));
    if (!input->buffer) {
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

int_fast32_t ncsh_readline_putchar(char character, struct ncsh_Input* input)
{
    char temp_character;

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
            return EXIT_CONTINUE;
        }

        while (input->pos > input->start_pos + 1) {
            ncsh_write_literal(MOVE_CURSOR_LEFT);
            --input->pos;
        }
    }
    else { // end of line insertions
        putchar(character);
        fflush(stdout);
        input->buffer[input->pos++] = character;

        if (input->pos >= input->max_pos) {
            input->max_pos = input->pos;
            input->buffer[input->pos] = '\0';
        }
    }

    return EXIT_SUCCESS;
}

eskilib_nodiscard int_fast32_t ncsh_readline(struct ncsh_Input* input)
{
    char character;
    int exit = EXIT_SUCCESS;
    int result = EXIT_SUCCESS;

    input->reprint_prompt = true;
    ncsh_terminal_init();

    while (1) {
        if (input->reprint_prompt == true) {
	    if (ncsh_readline_prompt(input) == EXIT_FAILURE) {
                exit = EXIT_FAILURE;
		break;
            }
            input->history_position = 0;
            input->reprint_prompt = false;
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
            int_fast32_t tab_autocomplete_result = ncsh_readline_tab_autocomplete(input, input->autocompletions_tree);
	    if (tab_autocomplete_result != EXIT_SUCCESS) {
		exit = tab_autocomplete_result;
		break;
	    }

	    input->reprint_prompt = true;
            input->buffer[0] = '\0';
            input->pos = 0;
            input->max_pos = 0;

            continue;
        }
        else if (character == BACKSPACE_KEY) {
            if (ncsh_readline_backspace(input) == EXIT_FAILURE) {
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
                    if (!input->pos && !input->max_pos) {
                        continue;
                    }
                    /*if (!input->current_autocompletion[0]) {
                        continue;
                    }*/

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
                    if (!input->pos || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
                        continue;
                    }

                    ncsh_write_literal(MOVE_CURSOR_LEFT);

                    --input->pos;

                    break;
                }
                case UP_ARROW: {
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
			goto exit;
                    }

                    if (character != DELETE_KEY) {
                        continue;
                    }

                    if (ncsh_readline_delete(input) == EXIT_FAILURE) {
                        key_result = EXIT_FAILURE;
			goto exit;
                    }

                    break;
                }
                case HOME_KEY: {
                    if (!input->pos) {
                        continue;
                    }

                    ncsh_write_literal(RESTORE_CURSOR_POSITION);
                    input->pos = 0;

                    break;
                }
                case END_KEY: {
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
        }
        else if (character == '\n' || character == '\r') {
            if (!input->pos && !input->buffer[input->pos]) {
                input->reprint_prompt = true;
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

            result = ncsh_readline_putchar(character, input);
            switch (result) {
            case EXIT_CONTINUE: {
                continue;
            }
            case EXIT_FAILURE: {
                exit = EXIT_FAILURE;
                goto exit;
            }
            }
        }

        ncsh_readline_autocomplete(input);
    }

exit:
    ncsh_terminal_reset();
    return exit;
}

void ncsh_readline_exit(struct ncsh_Input* input)
{
    free(input->buffer);
    ncsh_history_exit(&input->history);
    free(input->current_autocompletion);
    ncsh_autocompletions_free(input->autocompletions_tree);
    ncsh_terminal_reset();
}
