#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "eskilib/eskilib_colors.h"
#include "ncsh_autocompletions.h"
#include "ncsh_defines.h"
#include "ncsh_terminal.h"
#include "ncsh_types.h"
#include "ncsh_readline.h"

eskilib_nodiscard int_fast32_t ncsh_backspace(struct ncsh_Input* input)
{
    if (input->pos == 0) {
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
        if (input->pos == 0 || !input->buffer[input->pos - 1]) {
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

    if (input->pos == 0) {
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

void ncsh_autocomplete(struct ncsh_Input* input, struct ncsh_Shell* shell)
{
    uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(
        input->buffer, input->pos + 1, shell->current_autocompletion, shell->autocompletions_tree);

    if (autocompletions_matches_count == 0) {
        shell->current_autocompletion[0] = '\0';
        return;
    }

    int len = strlen(shell->current_autocompletion);
    shell->terminal.position = ncsh_terminal_position();
    int lines_before = shell->terminal.lines.y;
    int lines_after = lines_before;
    // printf("x: %d\n", shell->terminal.lines.x + (int)input->pos + len - 1);
    if (shell->terminal.lines.x + (int)input->pos + len - 1 > shell->terminal.size.x) {
	++lines_after;
    }
    // ncsh_terminal_line_size(input->pos, &shell->terminal);

    if (lines_before != lines_after) {
        /*if (write(STDOUT_FILENO, WHITE_DIM, sizeof(WHITE_DIM) - 1) == -1)
            return;

        for (int_fast32_t i = 0; i < shell->terminal.size.x - shell->terminal.position.x; ++i) {
            putchar(shell->current_autocompletion[i]);
        }
        putchar('\n');
        for (int_fast32_t i = shell->terminal.size.x - shell->terminal.position.x; i < length; ++i) {
            putchar(shell->current_autocompletion[i]);
        }

        if (write(STDOUT_FILENO, RESET, sizeof(RESET) - 1) == -1)
            return;
        fflush(stdout);*/
        return;
    }

    printf(WHITE_DIM "%s" RESET, shell->current_autocompletion);
    ncsh_terminal_move(shell->terminal.position.x, shell->terminal.position.y);
    fflush(stdout);
}

char ncsh_read(void)
{
    char character = 0;
    if (read(STDIN_FILENO, &character, 1) == 0) {
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

    if (autocompletions_matches_count == 0) {
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
            if (position == 0) {
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

eskilib_nodiscard int_fast32_t ncsh_readline(struct ncsh_Input* input, struct ncsh_Shell* shell)
{
    char character;
    char temp_character;

    while (1) {
        if (input->pos == 0 && shell->terminal.prompt.reprint == true) {
            if (ncsh_terminal_prompt(&shell->terminal) == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            shell->history_position = 0;
        }
        else {
            shell->terminal.prompt.reprint = true;
        }

        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            return EXIT_FAILURE;
        }

        if (character == CTRL_D) {
            ncsh_write_literal(ERASE_CURRENT_LINE);
            putchar('\n');
            fflush(stdout);
            return EXIT_SUCCESS_END;
        }
        else if (character == CTRL_W) { // delete last word
            shell->terminal.prompt.reprint = false;
            if (input->pos == 0 && input->max_pos == 0) {
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
            shell->args.count = 0;
            shell->args.values[0] = NULL;
        }
        else if (character == CTRL_U) { // delete entire line
            shell->terminal.prompt.reprint = false;
            if (input->pos == 0 && input->max_pos == 0) {
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
                shell->terminal.prompt.reprint = true;
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
            shell->terminal.prompt.reprint = false;
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
                    shell->terminal.prompt.reprint = false;
                    if (input->pos == 0 && input->max_pos == 0) {
                        continue;
                    }

                    if (input->pos == input->max_pos && input->buffer[0]) {
                        printf("%s", shell->current_autocompletion);
                        for (uint_fast32_t i = 0; shell->current_autocompletion[i] != '\0'; i++) {
                            input->buffer[input->pos] = shell->current_autocompletion[i];
                            ++input->pos;
                        }
                        input->buffer[input->pos] = '\0';

                        if (input->pos > input->max_pos) {
                            input->max_pos = input->pos;
                        }

                        fflush(stdout);
                        shell->current_autocompletion[0] = '\0';
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
                    shell->terminal.prompt.reprint = false;

                    if (input->pos == 0 || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
                        continue;
                    }

                    ncsh_write_literal(MOVE_CURSOR_LEFT);

                    --input->pos;

                    break;
                }
                case UP_ARROW: {
                    shell->terminal.prompt.reprint = false;

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
                    shell->terminal.prompt.reprint = false;

                    if (shell->history_position == 0) {
                        continue;
                    }

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

                    if (character != DELETE_KEY) {
                        continue;
                    }

                    shell->terminal.prompt.reprint = false;

                    if (ncsh_delete(input) == EXIT_FAILURE) {
                        return EXIT_FAILURE;
                    }

                    break;
                }
                case HOME_KEY: {
                    shell->terminal.prompt.reprint = false;
                    if (input->pos == 0) {
                        continue;
                    }

                    ncsh_write_literal(RESTORE_CURSOR_POSITION);
                    input->pos = 0;

                    break;
                }
                case END_KEY: {
                    shell->terminal.prompt.reprint = false;
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

                if (input->pos > input->max_pos) {
                    input->max_pos = input->pos;
                }

                if (input->pos == input->max_pos) {
                    input->buffer[input->pos] = '\0';
                }

                fflush(stdout);

                if (input->pos == 0 || input->buffer[1] == '\0') {
                    continue;
                }

                while (input->pos > input->start_pos + 1) {
                    ncsh_write_literal(MOVE_CURSOR_LEFT);
                    --input->pos;
                }
            }
            else { // end of line insertions
	        assert(input->pos <= INT_MAX);
		if ((shell->terminal.lines.x + (int)input->pos / shell->terminal.lines.y) > shell->terminal.size.x) {
		    ++shell->terminal.lines.y;
		    // shell->terminal.lines.x = shell->terminal.lines.x + (int)input->pos - shell->terminal.size.x;
		    putchar('\n');
		}
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

        ncsh_autocomplete(input, shell);
    }
}
