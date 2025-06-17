/* Copyright ncsh (C) by Alex Eski 2025 */
/* ncreadline.c: read input, handle prompts, history, and autocompletions */
/* Internal functions use ncrl prefix, external functions use ncreadline */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../config.h"
#include "../configurables.h"
#include "../defines.h"
#include "../eskilib/ecolors.h"
#include "../eskilib/eresult.h"
#include "ac.h"
#include "ncreadline.h"
#include "terminal.h"
#include "prompt.h"

#ifdef NCSH_RL_TEST
#include "ncreadline_mocks.h"
#endif

extern volatile int sigwinch_caught;

// IO
/* enum Line_Adjustment
 * Represents if the cursor was moved to previous line, next line, or not at all.
 */
enum Line_Adjustment : uint8_t {
    L_NONE = 0,
    L_NEXT,
    L_PREVIOUS
};

/* ncrl_is_end_of_line
 * Determines if the cursor is currently at the end of the current line.
 * Returns: true if at end of current line.
 */
[[nodiscard]]
bool ncrl_is_end_of_line(Input* rst input)
{
    if (input->lines_y == 0) {
        input->lines_x[0] = input->pos;
        return input->pos + input->prompt_len >= (size_t)input->terminal_size.x;
    }

    int current_line_pos = input->pos;
    for (int i = 0; i < input->lines_y; ++i) {
        current_line_pos -= input->lines_x[i];
    }

    if (current_line_pos < 0) {
        return false;
    }
    input->lines_x[input->lines_y] = current_line_pos;
    return current_line_pos >= input->terminal_size.x;
}

int ncrl_resize(Input* rst input)
{
    // need to reset saved cursor position as well on resize
    // use previous size
    // Coordinates prev_size = input->terminal_size;

    input->terminal_size = terminal_size();
    input->lines_y = 0;
    size_t len = input->pos;
    while (len) {
        if (len < (size_t)input->terminal_size.x) {
            input->lines_x[input->lines_y] = len;
            break;
        }
        input->lines_x[input->lines_y] = input->terminal_size.x;
        ++input->lines_y;
        len -= (size_t)input->terminal_size.x;
    }
    input->current_y = input->lines_y;

    return EXIT_SUCCESS;
}

/* ncrl_adjust_line_if_needed
 * Checks if a newline needs to be inserted.
 * For moving to the next line, nothing happens except increasing lines_y and current_y, which track y position relative
 * to the line the prompt started on. For moving to the previous line, we manually move the cursor to the end of the
 * previous line, decrease lines_y and current_y. Returns: enum Line_Adjustment, a value that indicates whether any line
 * change has happened or not
 */
enum Line_Adjustment ncrl_adjust_line_if_needed(Input* rst input)
{
    if (!input->pos || input->pos < (size_t)input->terminal_size.x) {
        return L_NONE;
    }

    if (ncrl_is_end_of_line(input)) {
        if (input->lines_y == 0) {
            input->lines_x[0] -= 1;
        }
        ++input->lines_y;
        input->current_y = input->lines_y;
        return L_NEXT;
    }

    // is start of line?
    if (input->lines_y > 0 && input->lines_x[input->lines_y] == 0) {
        --input->lines_y;
        input->current_y = input->lines_y;
        terminal_move_to_end_of_previous_line();
        fflush(stdout);
        return L_PREVIOUS;
    }

    return L_NONE;
}

/* ncrl_backspace
 * Handles backspace key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 * Returns: EXIT_SUCCESS OR EXIT_FAILURE
 */
[[nodiscard]]
int ncrl_backspace(Input* rst input)
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

/* ncrl_delete
 * Handles delete key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 * Returns: EXIT_SUCCESS OR EXIT_FAILURE
 */
[[nodiscard]]
int ncrl_delete(Input* rst input)
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

[[nodiscard]]
int ncrl_line_delete(Input* rst input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    ncsh_write_literal(RESTORE_CURSOR_POSITION);
    ncsh_write_literal(ERASE_BELOW);
    fflush(stdout);

    memset(input->buffer, '\0', input->max_pos + 1);
    input->max_pos = 0;
    input->pos = 0;
    memset(input->lines_x, 0, (size_t)input->lines_y + 1);
    input->lines_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_word_delete(Input* rst input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);
    input->buffer[input->pos] = '\0';
    --input->pos;

    // while (input->pos > 0 && input->buffer[input->pos] != ' ') {
    while (input->pos > 0) {
        if (ncrl_adjust_line_if_needed(input) == L_PREVIOUS) {
            ncsh_write_literal(ERASE_CURRENT_LINE);
            fflush(stdout);
        }
        // if (input->buffer[input->pos] == ' ' || (input->pos > 0 && input->buffer[input-> pos - 1] == ' '))
        if (input->buffer[input->pos] == ' ')
            break;
        ncsh_write_literal(BACKSPACE_STRING);
        input->buffer[input->pos] = '\0';
        --input->pos;
        /*if (input->buffer[input->pos] == ' ')
            break;*/
    }
    fflush(stdout);

    input->max_pos = input->pos;

    return EXIT_SUCCESS;
}

/*[[nodiscard]]
int ncrl_line_reset(Input* rst input)
{
    if (input->current_autocompletion[0] == '\0') {
    return EXIT_SUCCESS;
    }

    // deletes chars, but doesn't work, prevents line wrap
    // size_t len = strlen(current_autocompletion);
    // terminal_characters_delete(len);
    // fflush(stdout);

    // deletes chars, but doesn't work, prevents line wrap
    // ncsh_write_literal(ERASE_CURRENT_LINE);

    fflush(stdout);
    ncsh_write_literal(ERASE_CURRENT_LINE);
    return EXIT_SUCCESS;
}*/

[[nodiscard]]
int ncrl_autocomplete(Input* rst input, Arena* rst scratch)
{
    if (!input->buffer[0] || input->buffer[input->pos] != '\0') {
        return EXIT_SUCCESS;
    }

    uint8_t ac_matches_count =
        ac_first(input->buffer, input->current_autocompletion, input->autocompletions_tree, *scratch);

    if (!ac_matches_count) {
        if (input->current_autocompletion[0] == '\0') {
            return EXIT_SUCCESS;
        }

        // deletes chars, but doesn't work, prevents line wrap
        // size_t len = strlen(current_autocompletion);
        // terminal_characters_delete(len);
        // fflush(stdout);

        // deletes chars, but doesn't work, prevents line wrap
        // ncsh_write_literal(ERASE_CURRENT_LINE);

        fflush(stdout);
        ncsh_write_literal(ERASE_CURRENT_LINE);
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }

    input->current_autocompletion_len = strlen(input->current_autocompletion);
    if (input->current_y == 0 &&
        input->prompt_len + (size_t)input->lines_x[input->lines_y] + input->current_autocompletion_len >
            (size_t)input->terminal_size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }
    else if ((size_t)input->lines_x[input->lines_y] + input->current_autocompletion_len >
             (size_t)input->terminal_size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }

    printf(ERASE_CURRENT_LINE WHITE_DIM "%s" RESET, input->current_autocompletion);
    /*int start_y = input->lines_y;
    if (start_y == 0) {
        int new_len = input->prompt_len + input->pos + current_autocompletion_len;
        for (int i = 0; new_len > input->terminal_size.x; ++i) {
            input->lines_x[i] = input->terminal_size.x - input->prompt_len;
            ++input->lines_y;
            new_len -= input->terminal_size.x;
        }
    }*/
    // if (current_autocompletion_len)
    // ncsh_write_literal(ERASE_CURRENT_LINE WHITE_DIM);
    /*char* current_autocompletion = input->current_autocompletion;
    while (!*current_autocompletion) {
    putchar(*current_autocompletion++);
    }
    fflush(stdout);*/
    // ncsh_write(input->current_autocompletion, strlen(input->current_autocompletion));
    // ncsh_write_literal(RESET);
    terminal_move_left(input->current_autocompletion_len);
    fflush(stdout);
    return EXIT_SUCCESS;
}

/* ncrl_autocomplete_select
 * render the current autocompletion.
 */
[[nodiscard]]
int ncrl_autocomplete_select(Input* rst input)
{
    printf("%s", input->current_autocompletion);
    for (size_t i = 0; input->current_autocompletion[i] != '\0'; i++) {
        input->buffer[input->pos] = input->current_autocompletion[i];
        ++input->pos;
    }
    input->buffer[input->pos] = '\0';

    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    fflush(stdout);
    input->current_autocompletion[0] = '\0';
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_right(Input* rst input)
{
    if (input->pos == NCSH_MAX_INPUT - 1 || (!input->buffer[input->pos] && !input->buffer[input->pos + 1])) {
        return EXIT_SUCCESS;
    }

    ncsh_write_literal(MOVE_CURSOR_RIGHT);

    ++input->pos;
    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_right_arrow_process(Input* rst input)
{
#if NCSH_AUTOCOMPLETION_KEY == NCSH_AUTOCOMPLETION_KEY_RIGHT_ARROW
    if (input->pos == input->max_pos && input->buffer[0]) {
        if (ncrl_autocomplete_select(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif // NCSH_AUTOCOMPLETION_KEY == NCSH_AUTOCOMPLETION_KEY_RIGHT_ARROW

    if (ncrl_move_cursor_right(input) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_history_up(Input* rst input)
{
    input->history_entry = history_get(input->history_position, &input->history);
    if (input->history_entry.length > 0) {
        ++input->history_position;
        ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        ncsh_write(input->buffer, input->pos);
        ncrl_adjust_line_if_needed(input);
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_history_down(Input* rst input)
{
    input->history_entry = history_get(input->history_position - 2, &input->history);

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

    ncrl_adjust_line_if_needed(input);

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_left(Input* rst input)
{
    ncsh_write_literal(MOVE_CURSOR_LEFT);
    --input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_home(Input* rst input)
{
    ncsh_write_literal(RESTORE_CURSOR_POSITION);
    input->pos = 0;
    input->current_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_end(Input* rst input)
{
    if (input->lines_y > input->current_y) {
        ncsh_write_literal(HIDE_CURSOR);
        input->current_y = input->lines_y - input->current_y;
        terminal_move_down(input->current_y);
        putchar(MOVE_CURSOR_START_OF_LINE_CHAR);
        fflush(stdout);
        terminal_move_right(input->lines_x[input->current_y] - 1);
        ncsh_write_literal(SHOW_CURSOR);
        fflush(stdout);
        input->pos = input->max_pos;
        return EXIT_SUCCESS;
    }

    while (input->buffer[input->pos]) {
        ncsh_write_literal(MOVE_CURSOR_RIGHT);
        ++input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
char ncrl_read()
{
    char character = 0;
    if (read(STDIN_FILENO, &character, 1) < 0) {
        perror(RED NCSH_ERROR_STDIN RESET);
        return EXIT_IO_FAILURE;
    }

    switch (character) {
    case ESCAPE_CHARACTER: {
        if (read(STDIN_FILENO, &character, 1) < 0) {
            perror(RED NCSH_ERROR_STDIN RESET);
            return EXIT_IO_FAILURE;
        }

        if (character == '[') {
            if (read(STDIN_FILENO, &character, 1) < 0) {
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

[[nodiscard]]
int ncrl_autocompletions_select_from(Input* rst input, Arena* rst scratch)
{
    /*#if NCSH_AUTOCOMPLETION_KEY == NCSH_AUTOCOMPLETION_KEY_TAB
        if (input->pos == input->max_pos && input->buffer[0]) {
            if (ncrl_autocomplete_select(input) != EXIT_SUCCESS) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
    #endif // NCSH_AUTOCOMPLETION_KEY == NCSH_AUTOCOMPLETION_KEY_TAB*/
    ncsh_write_literal(ERASE_CURRENT_LINE "\n");

    Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    int ac_matches_count = ac_get(input->buffer, autocompletion_matches, input->autocompletions_tree, *scratch);

    if (!ac_matches_count) {
        return EXIT_SUCCESS;
    }

    if (input->buffer) {
        for (int i = 0; i < ac_matches_count; ++i) {
            printf("%s%s\n", input->buffer, autocompletion_matches[i].value);
        }
    }
    else {
        for (int i = 0; i < ac_matches_count; ++i) {
            printf("%s\n", autocompletion_matches[i].value);
        }
    }

    terminal_move_up(ac_matches_count);
    fflush(stdout);

    int position = 0;
    char character;

    int exit = EXIT_SUCCESS;
    bool continue_input = true;
    while (continue_input) {
        if ((character = ncrl_read()) == EXIT_IO_FAILURE) {
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
            if (position == ac_matches_count - 1) {
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
            if (input->pos + length > NCSH_MAX_INPUT)
                return EXIT_FAILURE;
            char* input_buf = input->buffer + input->pos;
            if (!input_buf)
                return EXIT_FAILURE;
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

    terminal_move_down(ac_matches_count + 1 - position);
    if (input->buffer && exit == EXIT_SUCCESS_EXECUTE) {
        printf(YELLOW "%s\n" RESET, input->buffer);
    }
    fflush(stdout);

    return exit;
}

/* ncreadline_init
 * Allocates memory that lives for the lifetime of the shell and is used by ncrl to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int ncreadline_init(Config* rst config, Input* rst input, Arena* rst arena)
{
    input->user.value = getenv("USER");
    if (!input->user.value) {
        input->user.value = "";
        input->user.length = 1;
    }
    else {
        input->user.length = strlen(input->user.value) + 1;
    }
    input->buffer = arena_malloc(arena, NCSH_MAX_INPUT, char);

    if (history_init(config->config_location, &input->history, arena) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating data for and setting up history" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    input->current_autocompletion = arena_malloc(arena, NCSH_MAX_INPUT, char);
    input->autocompletions_tree = ac_alloc(arena);

    ac_add_multiple(input->history.entries, input->history.count, input->autocompletions_tree, arena);

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_putchar(char character, Input* rst input)
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

        for (size_t i = input->pos - 1; i < input->max_pos && i < NCSH_MAX_INPUT; ++i) {
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

    ncrl_adjust_line_if_needed(input);
    return EXIT_SUCCESS;
}

/*[[nodiscard]]
int ncrl_autocomplete_partial(Input* rst input, Arena* rst scratch)
{
    if (!input->buffer[0] || input->buffer[input->pos] != '\0') {
        return EXIT_SUCCESS;
    }

    if (input->current_autocompletion[0] == '\0' || !input->current_autocompletion_len) {
            return EXIT_SUCCESS;
    }

    int exit = ncrl_putchar(input->current_autocompletion[0], input);
    if (exit != EXIT_SUCCESS)
        return exit;

    for (size_t i = 1; i < input->current_autocompletion_len; ++i) {
        switch (input->current_autocompletion[i]) {
        case ' ': { goto exit; }
        case '\\': { goto exit; }
        case '/': { goto exit; }
        default: {
            exit = ncrl_putchar(input->current_autocompletion[i], input);
            if (exit != EXIT_SUCCESS)
                goto exit;
            break;
        }
        }
    }

exit:
    input->current_autocompletion[0] = '\0';
    input->current_autocompletion_len = 0;
    input->buffer[input->pos--] = '\0';
    fflush(stdout);
    ncsh_write_literal(ERASE_CURRENT_LINE);
    fflush(stdout);
    if (ncrl_autocomplete(input, scratch) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}*/

int ncrl_escape_char_process(Input* rst input)
{
    char character;
    if (read(STDIN_FILENO, &character, 1) < 0) {
        perror(RED NCSH_ERROR_STDIN RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    if (character != '[') {
        return EXIT_SUCCESS;
    }

    if (read(STDIN_FILENO, &character, 1) < 0) {
        perror(RED NCSH_ERROR_STDIN RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    switch (character) {
    case RIGHT_ARROW: {
        if (!input->pos && !input->max_pos) {
            return EXIT_CONTINUE;
        }

        if (ncrl_right_arrow_process(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    case LEFT_ARROW: {
        if (!input->pos || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
            return EXIT_CONTINUE;
        }

        if (ncrl_move_cursor_left(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    case UP_ARROW: {
        if (ncrl_history_up(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_CONTINUE;
    }
    case DOWN_ARROW: {
        if (!input->history_position) {
            return EXIT_CONTINUE;
        }

        if (ncrl_history_down(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_CONTINUE;
    }
    case DELETE_PREFIX_KEY: {
        if (read(STDIN_FILENO, &character, 1) < 0) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            return EXIT_FAILURE;
        }

        if (character != DELETE_KEY) {
            return EXIT_CONTINUE;
        }

        if (ncrl_delete(input) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    case HOME_KEY: {
        if (!input->pos) {
            return EXIT_CONTINUE;
        }

        if (ncrl_move_cursor_home(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    case END_KEY: {
        if (input->pos == input->max_pos) {
            return EXIT_CONTINUE;
        }

        if (ncrl_move_cursor_end(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    }
    return EXIT_SUCCESS;
}

int ncrl_char_process(char character, Input* rst input, Arena* rst scratch)
{
    int exit;
    switch (character) {
    case CTRL_D: { // exit
        ncsh_write_literal(ERASE_CURRENT_LINE);
        putchar('\n');
        fflush(stdout);
        return EXIT_SUCCESS_END;
    }
    case CTRL_W: { // delete last word
        if (ncrl_word_delete(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        return EXIT_CONTINUE;
    }
    case CTRL_U: { // delete entire line
        if (ncrl_line_delete(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        return EXIT_CONTINUE;
    }
    case TAB_KEY:
    case CTRL_A: {
        exit = ncrl_autocompletions_select_from(input, scratch);
        if (exit != EXIT_SUCCESS) {
            return exit;
        }

        input->reprint_prompt = true;
        input->buffer[0] = '\0';
        input->pos = 0;
        input->max_pos = 0;

        return EXIT_CONTINUE;
    }
    /*case TAB_KEY: {
        exit = ncrl_autocomplete_partial(input, scratch);
        if (exit != EXIT_SUCCESS) {
            return exit;
        }

        return EXIT_CONTINUE;
    }*/
    case BACKSPACE_KEY: {
        if (ncrl_backspace(input) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    case ESCAPE_CHARACTER: {
        return ncrl_escape_char_process(input);
    }
    case '\r':
    case '\n': {
        if (!input->pos && !input->buffer[input->pos]) {
            input->reprint_prompt = true;
            putchar('\n');
            fflush(stdout);
            return EXIT_CONTINUE;
        }

        while (input->pos < input->max_pos && input->buffer[input->pos]) {
            ++input->pos;
            ncsh_write_literal(MOVE_CURSOR_RIGHT);
        }

        while (input->pos > 1 && input->buffer[input->pos - 1] == ' ') {
            --input->pos;
        }
        ++input->pos;

        ncsh_write_literal(ERASE_CURRENT_LINE "\n");
        fflush(stdout);
        return EXIT_SUCCESS_EXECUTE;
    }
    default: {
        if (input->pos == NCSH_MAX_INPUT - 1) {
            fputs(RED "\nncsh: Hit max input.\n" RESET, stderr);
            input->buffer[0] = '\0';
            input->pos = 0;
            input->max_pos = 0;
            return EXIT_CONTINUE;
        }

        return ncrl_putchar(character, input);
    }
    }

    return EXIT_SUCCESS;
}

/* ncreadline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and
 * other inputs. Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
[[nodiscard]]
int ncreadline(Input* rst input, Arena* rst scratch)
{
    char character;
    int exit = EXIT_SUCCESS;

    input->reprint_prompt = true;
    input->terminal_size = terminal_init();

    while (1) {
        if (prompt_if_needed(input) != EXIT_SUCCESS) {
            exit = EXIT_FAILURE;
            break;
        }

        if (read(STDIN_FILENO, &character, 1) < 0) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            exit = EXIT_FAILURE;
            break;
        }

        exit = ncrl_char_process(character, input, scratch);
        switch (exit) {
        case EXIT_SUCCESS: {
            break;
        }
        case EXIT_SUCCESS_EXECUTE: {
            goto exit;
        }
        case EXIT_CONTINUE: {
            continue;
        }
        default: {
            goto exit;
        }
        }

        if (sigwinch_caught) {
            if (ncrl_resize(input) != EXIT_SUCCESS) {
                exit = EXIT_FAILURE;
                break;
            }
            sigwinch_caught = 0;
        }

        if (ncrl_autocomplete(input, scratch) != EXIT_SUCCESS) {
            exit = EXIT_FAILURE;
            break;
        }
    }

exit:
    terminal_reset();
    return exit;
}

/* ncreadline_exit
 * Saves history changes and restores the terminal settings from before the shell was started.
 */
void ncreadline_exit(Input* rst input)
{
    if (input && input->history.file && input->history.entries) {
        history_save(&input->history);
    }
    terminal_reset();
}
