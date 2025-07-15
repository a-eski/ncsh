/* Copyright ncsh (C) by Alex Eski 2025 */
/* ncreadline.c: read input, handle prompts, history, and autocompletions */
/* Internal functions use ncrl prefix, external functions use ncreadline */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../config.h"
#include "../configurables.h"
#include "../defines.h"
#include "../eskilib/eresult.h"
#include "../ttyterm/ttyterm.h"
#include "ac.h"
#include "ncreadline.h"
#include "prompt.h"
#include "input.h"

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
        return input->pos + input->prompt_len >= term.size.x;
    }

    size_t current_line_pos = input->pos;
    for (size_t i = 0; i < input->lines_y; ++i) {
        assert(input->lines_x[i] >= current_line_pos);
        current_line_pos -= input->lines_x[i];
    }

    input->lines_x[input->lines_y] = current_line_pos;
    return (size_t)current_line_pos >= term.size.x;
}

int ncrl_resize(Input* rst input)
{
    // need to reset saved cursor position as well on resize
    // use previous size
    // Coordinates prev_size = input->terminal_size;

    input->lines_y = 0;
    size_t len = input->pos;
    while (len) {
        if (len < term.size.x) {
            input->lines_x[input->lines_y] = len;
            break;
        }
        input->lines_x[input->lines_y] = term.size.x;
        ++input->lines_y;
        len -= term.size.x;
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
    if (!input->pos || input->pos < term.size.x) {
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

        term_goto_prev_eol();
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

    term_send(&tcaps.bs);
    term_send(&tcaps.line_clr_to_eol);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);
    input->buffer[input->max_pos] = '\0';

    while (input->buffer[input->pos] != '\0') {
        term_write(&input->buffer[input->pos], 1);
        ++input->pos;
    }

    while (input->pos > input->start_pos) {
        if (!input->pos || !input->buffer[input->pos - 1]) {
            break;
        }

        term_send(&tcaps.cursor_left);
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
    term_send(&tcaps.del);
    term_send(&tcaps.line_clr_to_eol);

    input->start_pos = input->pos;
    memmove(input->buffer + input->pos, input->buffer + input->pos + 1, input->max_pos);

    if (input->max_pos > 0) {
        --input->max_pos;
    }

    while (input->pos < input->max_pos && input->buffer[input->pos]) {
        term_write(&input->buffer[input->pos], 1);
        ++input->pos;
    }

    if (!input->pos) {
        return EXIT_SUCCESS;
    }

    while (input->pos > input->start_pos && input->pos != 0 && input->buffer[input->pos - 1]) {
        term_send(&tcaps.cursor_left);
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

    term_send(&tcaps.cursor_restore);
    term_send(&tcaps.scr_clr_to_eos);

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

    term_send(&tcaps.bs);
    term_send(&tcaps.line_clr_to_eol);
    input->buffer[input->pos] = '\0';
    --input->pos;

    // while (input->pos > 0 && input->buffer[input->pos] != ' ') {
    while (input->pos > 0) {
        if (ncrl_adjust_line_if_needed(input) == L_PREVIOUS) {
            term_send(&tcaps.line_clr_to_eol);
        }
        // if (input->buffer[input->pos] == ' ' || (input->pos > 0 && input->buffer[input-> pos - 1] == ' '))
        if (input->buffer[input->pos] == ' ')
            break;

        term_send(&tcaps.bs);
        input->buffer[input->pos] = '\0';
        --input->pos;
        /*if (input->buffer[input->pos] == ' ')
            break;*/
    }

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

        term_send(&tcaps.line_clr_to_eol);
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }

    input->current_autocompletion_len = strlen(input->current_autocompletion);
    if (input->current_y == 0 &&
        input->prompt_len + (size_t)input->lines_x[input->lines_y] + input->current_autocompletion_len >
            term.size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }
    else if ((size_t)input->lines_x[input->lines_y] + input->current_autocompletion_len >
             term.size.x) {
        input->current_autocompletion[0] = '\0';
        input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }

    term_send(&tcaps.line_clr_to_eol);
    term_color_set(244);
    term_print(input->current_autocompletion);
    term_send(&tcaps.color_reset);
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
    term_send_n(&tcaps.cursor_left, input->current_autocompletion_len);
    return EXIT_SUCCESS;
}

/* ncrl_autocomplete_select
 * render the current autocompletion.
 */
[[nodiscard]]
int ncrl_autocomplete_select(Input* rst input)
{
    term_print(input->current_autocompletion);
    for (size_t i = 0; input->current_autocompletion[i] != '\0'; i++) {
        input->buffer[input->pos] = input->current_autocompletion[i];
        ++input->pos;
    }
    input->buffer[input->pos] = '\0';

    if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }

    input->current_autocompletion[0] = '\0';
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_right(Input* rst input)
{
    if (input->pos == NCSH_MAX_INPUT - 1 || (!input->buffer[input->pos] && !input->buffer[input->pos + 1])) {
        return EXIT_SUCCESS;
    }

    term_send(&tcaps.cursor_right);

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
        term_send(&tcaps.cursor_restore);
        term_send(&tcaps.line_clr_to_eol);

        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        term_write(input->buffer, input->pos);
        ncrl_adjust_line_if_needed(input);
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_history_down(Input* rst input)
{
    input->history_entry = history_get(input->history_position - 2, &input->history);

    term_send(&tcaps.cursor_restore);
    term_send(&tcaps.line_clr_to_eol);

    if (input->history_entry.length > 0) {
        --input->history_position;
        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        term_write(input->buffer, input->pos);
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
    term_send(&tcaps.cursor_left);
    --input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_home(Input* rst input)
{
    term_send(&tcaps.cursor_restore);
    input->pos = 0;
    input->current_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int ncrl_move_cursor_end(Input* rst input)
{
    if (input->lines_y > input->current_y) {
        term_send(&tcaps.cursor_hide);
        input->current_y = input->lines_y - input->current_y;
        assert(input->current_y > 0);
        term_send_n(&tcaps.cursor_down, input->current_y);
        term_send(&tcaps.line_goto_bol);
        term_send_n(&tcaps.cursor_right, input->lines_x[input->current_y] - 1);
        term_send(&tcaps.cursor_show);
        input->pos = input->max_pos;
        return EXIT_SUCCESS;
    }

    while (input->buffer[input->pos]) {
        term_send(&tcaps.cursor_right);
        ++input->pos;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
char ncrl_read()
{
    char character = 0;
    if (read(STDIN_FILENO, &character, 1) < 0) {
        term_perror(NCSH_ERROR_STDIN);
        return EXIT_IO_FAILURE;
    }

    switch (character) {
    case ESCAPE_CHARACTER: {
        if (read(STDIN_FILENO, &character, 1) < 0) {
            term_perror(NCSH_ERROR_STDIN);
            return EXIT_IO_FAILURE;
        }

        if (character == '[') {
            if (read(STDIN_FILENO, &character, 1) < 0) {
                term_perror(NCSH_ERROR_STDIN);
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
    term_send(&tcaps.line_clr_to_eol);
    term_send(&tcaps.newline);

    Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    uint8_t ac_matches_count = ac_get(input->buffer, autocompletion_matches, input->autocompletions_tree, *scratch);

    if (!ac_matches_count) {
        return EXIT_SUCCESS;
    }

    if (input->buffer) {
        for (int i = 0; i < ac_matches_count; ++i) {
            term_println("%s%s", input->buffer, autocompletion_matches[i].value);
        }
    }
    else {
        for (int i = 0; i < ac_matches_count; ++i) {
            term_println("%s", autocompletion_matches[i].value);
        }
    }

    term_send_n(&tcaps.cursor_up, ac_matches_count);

    size_t position = 0;
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
            term_send(&tcaps.cursor_up);
            --position;
            break;
        }
        case DOWN_ARROW: {
            if (position == (size_t)(ac_matches_count - 1)) {
                break;
            }
            term_send(&tcaps.cursor_down);
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

    term_send_n(&tcaps.cursor_down, ac_matches_count + 1 - position);
    if (input->buffer && exit == EXIT_SUCCESS_EXECUTE) {
        term_color_set(220);
        term_println(input->buffer);
        term_send(&tcaps.color_reset);
    }

    return exit;
}

void ncrl_prompt_init()
{
    bool show_user = true;
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    show_user = true;
#elif NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NONE
    show_user = false;
#endif /* NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    enum Dir_Type dir_type = DIR_SHORT;

#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    dir_type = DIR_NORMAL;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    dir_type = DIR_SHORT;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    dir_type = DIR_NONE;
#endif /* NCSH_PROMPT_DIRECTORY == NCSH_PROMPT_DIRECTORY */

    prompt_init(show_user, dir_type);
}

/* ncreadline_init
 * Allocates memory that lives for the lifetime of the shell and is used by ncrl to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in defines.h (EXIT_...)
 */
int ncreadline_init(Config* rst config, Input* rst input, Arena* rst arena)
{
    ncrl_prompt_init();

    input->user.value = getenv("USER");
    if (!input->user.value) {
        input->user.value = (char*)"";
        input->user.length = 1;
    }
    else {
        input->user.length = strlen(input->user.value) + 1;
    }
    input->buffer = arena_malloc(arena, NCSH_MAX_INPUT, char);

    if (history_init(config->config_location, &input->history, arena) != E_SUCCESS) {

        term_perror("ncsh: Error when allocating data for and setting up history");
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
            term_putc(character);
            character = temp_character;
            ++input->pos;
        }

        for (size_t i = input->pos - 1; i < input->max_pos && i < NCSH_MAX_INPUT; ++i) {
            temp_character = character;
            character = input->buffer[i + 1];
            input->buffer[i + 1] = temp_character;
            term_putc(temp_character);
            ++input->pos;
        }

        if (input->pos > input->max_pos) {
            input->max_pos = input->pos;
        }

        if (input->pos == input->max_pos) {
            input->buffer[input->pos] = '\0';
        }

        if (!input->pos || input->buffer[1] == '\0') {
            return EXIT_CONTINUE;
        }

        while (input->pos > input->start_pos + 1) {
            term_send(&tcaps.cursor_left);
            --input->pos;
        }
    }
    else { // end of line insertions
        term_putc(character);
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
        term_perror(NCSH_ERROR_STDIN);
        return EXIT_FAILURE;
    }

    if (character != '[') {
        return EXIT_SUCCESS;
    }

    if (read(STDIN_FILENO, &character, 1) < 0) {
        term_perror(NCSH_ERROR_STDIN);
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
            term_perror(NCSH_ERROR_STDIN);
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
    // Shift left arrow or shift right arrow
    // ^[[1;2C
    // ^[[1;2D
    case '[': {
        if (read(STDIN_FILENO, &character, 1) < 0) {
            term_perror(NCSH_ERROR_STDIN);
            return EXIT_FAILURE;
        }

        if (character != '1') {
            return EXIT_SUCCESS;
        }

        if (read(STDIN_FILENO, &character, 1) < 0) {
            term_perror(NCSH_ERROR_STDIN);
            return EXIT_FAILURE;
        }

        if (character != ';') {
            return EXIT_SUCCESS;
        }

        if (read(STDIN_FILENO, &character, 1) < 0) {
            term_perror(NCSH_ERROR_STDIN);
            return EXIT_FAILURE;
        }

        if (character != '2') {
            return EXIT_SUCCESS;
        }

        switch (character) {
        case RIGHT_ARROW: {
            term_puts("detected shift right arrow");
            break;
        }
        case LEFT_ARROW: {
            term_puts("detected shift left arrow");
            break;
        }
        }
    }
    }
    return EXIT_SUCCESS;
}

int ncrl_char_process(char character, Input* rst input, Arena* rst scratch)
{
    int exit;
    switch (character) {
    case CTRL_D: { // exit
        term_send(&tcaps.line_clr_to_eol);
        term_send(&tcaps.newline);
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
            term_send(&tcaps.newline);
            return EXIT_CONTINUE;
        }

        while (input->pos < input->max_pos && input->buffer[input->pos]) {
            ++input->pos;
            term_send(&tcaps.cursor_right);
        }

        while (input->pos > 1 && input->buffer[input->pos - 1] == ' ') {
            --input->pos;
        }
        ++input->pos;

        term_send(&tcaps.line_clr_to_eol);
        term_send(&tcaps.newline);
        return EXIT_SUCCESS_EXECUTE;
    }
    default: {
        if (input->pos == NCSH_MAX_INPUT - 1) {
            term_color_set(196);
            term_send(&tcaps.newline);
            term_fprintln(stderr, "ncsh: Hit max input.");
            term_send(&tcaps.color_reset);
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
    // input->terminal_size = terminal_init();

    while (1) {
        if (prompt_if_needed(input) != EXIT_SUCCESS) {
            exit = EXIT_FAILURE;
            break;
        }

        if (read(STDIN_FILENO, &character, 1) < 0) {
            term_perror(NCSH_ERROR_STDIN);
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
    // terminal_reset();
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
    term_reset();
}
