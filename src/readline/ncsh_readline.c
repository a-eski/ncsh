// Copyright (c) ncsh by Alex Eski 2025

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../eskilib/eskilib_colors.h"
#include "../eskilib/eskilib_result.h"
#include "../eskilib/eskilib_string.h"
#include "../ncsh_config.h"
#include "../ncsh_configurables.h"
#include "../ncsh_defines.h"
#include "ncsh_autocompletions.h"
#include "ncsh_terminal.h"
#include "ncsh_readline.h"

/* Prompt */
/* ncsh_readline_prompt_size
 * get the prompt size accounting for prompt length, user length, and cwd length
 * Returns: length of the prompt
 */
[[nodiscard]]
size_t ncsh_readline_prompt_size(const size_t user_len, const size_t dir_len)
{
    // shell prompt format:
    // {user} {directory} {symbol} {buffer}
    // user, directory include null termination, use as space for len
    //     {user}{space (\0)}      {directory}{space (\0) excluded by -1}     {>}  {space}     {buffer}
    if (user_len == 0 && dir_len == 0) {
        return NCSH_PROMPT_ENDING_STRING_LENGTH;
    }

    return user_len + dir_len - 1 + NCSH_PROMPT_ENDING_STRING_LENGTH;
}


/* This section is included at compile time if the prompt directory setting is set to use shortened directory. */
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT

/* ncsh_readline_prompt_short_directory_get
 * gets a shortened version of the cwd, the last 2 directories in the cwd.
 * i.e. /home/alex/dir becomes /alex/dir
 * Returns: length of the shortened cwd
 */
[[nodiscard]]
size_t ncsh_readline_prompt_short_directory_get(const char* const cwd, char* output)
{
    assert(cwd);

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

/* ncsh_readline_prompt_short_directory_print
 * Gets and prints the shortened directory prompt when it is enabled at compile time via ncsh_configurables.h option.
 * Handles whether User is included in prompt or not, based on
 * Returns: EXIT_FAILURE or EXIT_SUCCESS
 */
[[nodiscard]]
int_fast32_t ncsh_readline_prompt_short_directory_print(struct ncsh_Input* const restrict input)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    size_t dir_len = ncsh_readline_prompt_short_directory_get(cwd, directory);
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value, directory);
    input->prompt_len = ncsh_readline_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, directory);
    input->prompt_len = ncsh_readline_prompt_size(0, dir_len);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */
    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */

/* This section is included at compile time if the prompt directory setting is set to use the full directory. */
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL

[[nodiscard]]
int_fast32_t ncsh_readline_prompt_directory_print(struct ncsh_Input* const restrict input)
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
    input->prompt_len = ncsh_readline_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, cwd);
    input->prompt_len = ncsh_readline_prompt_size(0, dir_len);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);

    return EXIT_SUCCESS;
}
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL */

/* This section is included at compile time if the prompt directory setting is set to use the full directory. */
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE

[[nodiscard]]
int_fast32_t ncsh_readline_prompt_no_directory_print(struct ncsh_Input* const restrict input)
{
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value);
    input->prompt_len = ncsh_readline_prompt_size(input->user.length, 0);
#else
    printf(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);
    input->prompt_len = ncsh_readline_prompt_size(0, 0);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif

/* ncsh_readline_prompt
 * Prints the prompt based on the current prompt compile-time settings.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int_fast32_t ncsh_readline_prompt(struct ncsh_Input* const restrict input)
{
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    return ncsh_readline_prompt_short_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    return ncsh_readline_prompt_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    return ncsh_readline_prompt_no_directory_print(input);
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */
}

/* ncsh_readline_prompt_if_needed
 * Prints the prompt if input.reprint_prompt is true.
 * Calls ncsh_readline_prompt.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int_fast32_t ncsh_readline_prompt_if_needed(struct ncsh_Input* const restrict input)
{
    if (input->reprint_prompt == true) {
        if (ncsh_readline_prompt(input) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        input->lines_y = 0;
        input->history_position = 0;
        input->reprint_prompt = false;
    }
    return EXIT_SUCCESS;
}

// IO
/* enum ncsh_Line_Adjustment
 * Represents if the cursor was moved to previous line, next line, or not at all.
 */
enum ncsh_Line_Adjustment : uint_fast8_t {
    L_NONE = 0,
    L_NEXT,
    L_PREVIOUS
};

/* ncsh_readline_is_end_of_line
 * Determines if the cursor is currently at the end of the current line.
 * Returns: true if at end of current line.
 */
[[nodiscard]]
bool ncsh_readline_is_end_of_line(struct ncsh_Input* const restrict input)
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

/* ncsh_readline_is_start_of_line
 * Determines if the cursor is at the start of the current line.
 * Returns: true if at the start of the current line.
 */
[[nodiscard]]
bool ncsh_readline_is_start_of_line(const struct ncsh_Input* const restrict input)
{
    return input->lines_y > 0 && input->lines_x[input->lines_y] == 0;
}

/* ncsh_readline_adjust_line_if_needed
 * Checks if a newline needs to be inserted.
 * For moving to the next line, nothing happens except increasing lines_y and current_y, which track y position relative to the line the prompt started on.
 * For moving to the previous line, we manually move the cursor to the end of the previous line, decrease lines_y and current_y.
 * Returns: enum ncsh_Line_Adjustment, a value that indicates whether any line change has happened or not
 */
enum ncsh_Line_Adjustment ncsh_readline_adjust_line_if_needed(struct ncsh_Input* const restrict input)
{
    if (ncsh_readline_is_end_of_line(input)) {
	if (input->lines_y == 0) {
	    input->lines_x[0] -= 1;
	}
        ++input->lines_y;
	input->current_y = input->lines_y;
	return L_NEXT;
    }
    else if (ncsh_readline_is_start_of_line(input)) {
        --input->lines_y;
	input->current_y = input->lines_y;
        ncsh_terminal_move_to_end_of_previous_line();
        fflush(stdout);
	return L_PREVIOUS;
    }

    return L_NONE;
}

/* ncsh_readline_backspace
 * Handles backspace key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 * Returns: EXIT_SUCCESS OR EXIT_FAILURE
 */
[[nodiscard]]
int_fast32_t ncsh_readline_backspace(struct ncsh_Input* const restrict input)
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

/* ncsh_readline_delete
 * Handles delete key input in any position, end of line, midline, start of line.
 * Adjusts buffer and buffer position which holds user input.
 * Returns: EXIT_SUCCESS OR EXIT_FAILURE
 */
[[nodiscard]]
int_fast32_t ncsh_readline_delete(struct ncsh_Input* const restrict input)
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
int_fast32_t ncsh_readline_line_delete(struct ncsh_Input* const restrict input)
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
int_fast32_t ncsh_readline_word_delete(struct ncsh_Input* const restrict input)
{
    if (!input->pos && !input->max_pos) {
        return EXIT_SUCCESS;
    }

    ncsh_write_literal(BACKSPACE_STRING ERASE_CURRENT_LINE);
    input->buffer[input->pos] = '\0';
    --input->pos;

    while (input->pos > 0 && input->buffer[input->pos] != ' ') {
	if (ncsh_readline_adjust_line_if_needed(input) == L_PREVIOUS) {
	    ncsh_write_literal(ERASE_CURRENT_LINE);
	    fflush(stdout);
	}
        ncsh_write_literal(BACKSPACE_STRING);
        input->buffer[input->pos] = '\0';
        --input->pos;
    }
    fflush(stdout);

    input->max_pos = input->pos;

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_line_reset(struct ncsh_Input* const restrict input)
{
    if (input->current_autocompletion[0] == '\0') {
	return EXIT_SUCCESS;
    }
    // deletes chars, but doesn't work, prevents line wrap
    // size_t len = strlen(current_autocompletion);
    // ncsh_terminal_characters_delete(len);
    // fflush(stdout);

    // deletes chars, but doesn't work, prevents line wrap
    // ncsh_write_literal(ERASE_CURRENT_LINE);

    fflush(stdout);
    ncsh_write_literal(ERASE_CURRENT_LINE);

    /*ncsh_write_literal(HIDE_CURSOR);
    ncsh_terminal_move_right(input->current_autocompletion_len);
    fflush(stdout);
    for (size_t i = input->current_autocompletion_len; i > input->pos; --i) {
        ncsh_write_literal(BACKSPACE_STRING);
	fflush(stdout);
    }

    ncsh_terminal_move_left(input->current_autocompletion_len);
    ncsh_write_literal(SHOW_CURSOR);
    fflush(stdout);*/

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_autocomplete_print(struct ncsh_Input* const restrict input)
{
    printf(ERASE_CURRENT_LINE WHITE_DIM "%s" RESET, input->current_autocompletion);
    input->current_autocompletion_len = strlen(input->current_autocompletion);
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
    ncsh_terminal_move_left(input->current_autocompletion_len);
    fflush(stdout);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_autocomplete(struct ncsh_Input* const restrict input,
                                        struct ncsh_Arena* scratch_arena)
{
    if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0') {
        return EXIT_SUCCESS;
    }
    else if (input->buffer[0] < 32) { // exclude control characters from autocomplete
	if (ncsh_readline_line_reset(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        memset(input->buffer, '\0', input->max_pos);
        input->pos = 0;
        input->max_pos = 0;
        return EXIT_SUCCESS;
    }

    uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(
        input->buffer, input->pos + 1, input->current_autocompletion,
        input->autocompletions_tree, *scratch_arena);

    if (!autocompletions_matches_count) {
        if (ncsh_readline_line_reset(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
        input->current_autocompletion[0] = '\0';
        return EXIT_SUCCESS;
    }

    return ncsh_readline_autocomplete_print(input);
}

/* ncsh_readline_autocomplete_select
 * render the current autocompletion.
 */
[[nodiscard]]
int_fast32_t ncsh_readline_autocomplete_select(struct ncsh_Input* const restrict input)
{
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
    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_move_cursor_right(struct ncsh_Input* const restrict input)
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
int_fast32_t ncsh_readline_right_arrow_process(struct ncsh_Input* const restrict input)
{
    if (input->pos == input->max_pos && input->buffer[0]) {
        if (ncsh_readline_autocomplete_select(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }
    }

    if (ncsh_readline_move_cursor_right(input) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_history_up(struct ncsh_Input* const restrict input)
{
    input->history_entry = ncsh_history_get(input->history_position, &input->history);
    if (input->history_entry.length > 0) {
        ++input->history_position;
        ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

        input->pos = input->history_entry.length - 1;
        input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        ncsh_write(input->buffer, input->pos);
        ncsh_readline_adjust_line_if_needed(input);
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_history_down(struct ncsh_Input* const restrict input)
{
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

    ncsh_readline_adjust_line_if_needed(input);

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_move_cursor_left(struct ncsh_Input* const restrict input)
{
    ncsh_write_literal(MOVE_CURSOR_LEFT);
    --input->pos;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_move_cursor_home(struct ncsh_Input* const restrict input)
{
    ncsh_write_literal(RESTORE_CURSOR_POSITION);
    input->pos = 0;
    input->current_y = 0;
    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_move_cursor_end(struct ncsh_Input* const restrict input)
{
    if (input->lines_y > input->current_y) {
	ncsh_write_literal(HIDE_CURSOR);
	input->current_y = input->lines_y - input->current_y;
	ncsh_terminal_move_down(input->current_y);
	putchar(MOVE_CURSOR_START_OF_LINE_CHAR);
	fflush(stdout);
	ncsh_terminal_move_right(input->lines_x[input->current_y] - 1);
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

[[nodiscard]]
int_fast32_t ncsh_readline_tab_autocomplete(struct ncsh_Input* const restrict input,
                                            struct ncsh_Arena* const scratch_arena)
{
    ncsh_write_literal(ERASE_CURRENT_LINE "\n");

    struct ncsh_Autocompletion autocompletion_matches[NCSH_MAX_AUTOCOMPLETION_MATCHES] = {0};
    int autocompletions_matches_count =
        ncsh_autocompletions_get(input->buffer, input->pos + 1, autocompletion_matches,
                                 input->autocompletions_tree, *scratch_arena);

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

    ncsh_terminal_move_down(autocompletions_matches_count + 1 - position);
    if (input->buffer && exit == EXIT_SUCCESS_EXECUTE) {
        printf(ncsh_YELLOW "%s\n" RESET, input->buffer);
    }
    fflush(stdout);

    return exit;
}

/* ncsh_readline_init
 * Allocates memory that lives for the lifetime of the shell and is used by readline to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_readline_init(struct ncsh_Config* const restrict config,
                                struct ncsh_Input* const restrict input,
                                struct ncsh_Arena* const arena)
{
    input->user.value = getenv("USER");
    input->user.length = strlen(input->user.value) + 1;
    input->buffer = alloc(arena, NCSH_MAX_INPUT, char);
    if (!input->buffer) {
        return EXIT_FAILURE;
    }

    enum eskilib_Result result;
    if ((result = ncsh_history_init(config->config_location, &input->history, arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating data for and setting up history" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    input->current_autocompletion = alloc(arena, NCSH_MAX_INPUT, char);
    if (!input->current_autocompletion) {
        perror(RED "ncsh: Error when allocating data for autocompletion results" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    input->autocompletions_tree = ncsh_autocompletions_alloc(arena);
    if (!input->autocompletions_tree) {
        perror(RED "ncsh: Error when loading data from history as autocompletions" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    ncsh_autocompletions_add_multiple(input->history.entries, input->history.count, input->autocompletions_tree, arena);

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_readline_putchar(char character, struct ncsh_Input* const restrict input)
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

/* ncsh_readline
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and other inputs.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
[[nodiscard]]
int_fast32_t ncsh_readline(struct ncsh_Input* const restrict input,
                           struct ncsh_Arena* const scratch_arena)
{
    char character;
    int exit = EXIT_SUCCESS;
    int result = EXIT_SUCCESS;

    input->reprint_prompt = true;
    input->terminal_size = ncsh_terminal_init();

    while (1) {
        if (ncsh_readline_prompt_if_needed(input) != EXIT_SUCCESS) {
            exit = EXIT_FAILURE;
            break;
        }

        if (read(STDIN_FILENO, &character, 1) == -1) {
            perror(RED NCSH_ERROR_STDIN RESET);
            fflush(stderr);
            exit = EXIT_FAILURE;
	    break;
        }

        if (character == CTRL_D) { // exit the shell
            ncsh_write_literal(ERASE_CURRENT_LINE);
            putchar('\n');
            fflush(stdout);
	    puts("exit");
            exit = EXIT_SUCCESS_END;
	    break;
        }
        else if (character == CTRL_W) { // delete last word
            if (ncsh_readline_word_delete(input) != EXIT_SUCCESS) {
		exit = EXIT_FAILURE;
		break;
	    }
        }
        else if (character == CTRL_U) { // delete entire line
            if (ncsh_readline_line_delete(input) != EXIT_SUCCESS) {
                exit = EXIT_FAILURE;
                break;
            }
        }
        else if (character == TAB_KEY) {
	    if ((result = ncsh_readline_tab_autocomplete(input, scratch_arena)) != EXIT_SUCCESS) {
		exit = result;
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

                switch (character) {
                case RIGHT_ARROW: {
		    if (!input->pos && !input->max_pos) {
        		continue;
    		    }

                    if (ncsh_readline_right_arrow_process(input) != EXIT_SUCCESS) {
                        exit = EXIT_FAILURE;
                        goto exit;
                    }

                    break;
                }
                case LEFT_ARROW: {
                    if (!input->pos || (!input->buffer[input->pos] && !input->buffer[input->pos - 1])) {
                        continue;
                    }

                    if (ncsh_readline_move_cursor_left(input) != EXIT_SUCCESS) {
			exit = EXIT_FAILURE;
			goto exit;
		    }

                    break;
                }
                case UP_ARROW: {
                    if (ncsh_readline_history_up(input) != EXIT_SUCCESS) {
                        exit = EXIT_FAILURE;
                        goto exit;
                    }

                    continue;
                }
                case DOWN_ARROW: {
                    if (!input->history_position) {
                        continue;
                    }

                    if (ncsh_readline_history_down(input) != EXIT_SUCCESS) {
                        exit = EXIT_FAILURE;
                        goto exit;
                    }

                    continue;
                }
                case DELETE_PREFIX_KEY: {
                    if (read(STDIN_FILENO, &character, 1) == -1) {
                        perror(RED NCSH_ERROR_STDIN RESET);
                        fflush(stderr);
                        exit = EXIT_FAILURE;
			goto exit;
                    }

                    if (character != DELETE_KEY) {
                        continue;
                    }

                    if (ncsh_readline_delete(input) == EXIT_FAILURE) {
                        exit = EXIT_FAILURE;
			goto exit;
                    }

                    break;
                }
                case HOME_KEY: {
                    if (!input->pos) {
                        continue;
                    }

		    if (ncsh_readline_move_cursor_home(input) != EXIT_SUCCESS) {
			exit = EXIT_FAILURE;
			goto exit;
		    }

                    break;
                }
                case END_KEY: {
                    if (input->pos == input->max_pos) {
                        continue;
                    }

		    if (ncsh_readline_move_cursor_end(input) != EXIT_SUCCESS) {
			exit = EXIT_FAILURE;
			goto exit;
		    }

                    break;
                }
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

        ncsh_readline_adjust_line_if_needed(input);
        if (ncsh_readline_autocomplete(input, scratch_arena) != EXIT_SUCCESS) {
	    exit = EXIT_FAILURE;
	    break;
	}
    }

exit:
    ncsh_terminal_reset();
    return exit;
}

/* ncsh_readline_history_and_autocompletion_add
 * Add user input that was able to be processed and executed by the VM to readline's history and autocompletion data stores.
 */
void ncsh_readline_history_and_autocompletion_add(struct ncsh_Input* const restrict input,
                                                  struct ncsh_Arena* const arena)
{
    ncsh_history_add(input->buffer, input->pos, &input->history, arena);
    ncsh_autocompletions_add(input->buffer, input->pos, input->autocompletions_tree, arena);
}

/* ncsh_readline_exit
 * Saves history changes and restores the terminal settings from before the shell was started.
 */
void ncsh_readline_exit(struct ncsh_Input* const restrict input)
{
    ncsh_history_save(&input->history);
    ncsh_terminal_reset();
}
