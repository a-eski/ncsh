/* Copyright ncsh by Alex Eski 2025 */

#include <assert.h>
#include <limits.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/ncsh_arena.h>
#include <readline/ncsh_autocompletions.h>
#include <readline/ncsh_string.h>

#include <readline/readline.h>
#include <readline/ncsh_readline.h>
#include <readline/history.h>

#include "../eskilib/eskilib_colors.h"
#include "../eskilib/eskilib_result.h"
#include "../ncsh_config.h"
#include "../ncsh_configurables.h"
#include "../ncsh_defines.h"
#include "ncsh_terminal.h"
#include "ncsh_input.h"

/* Prompt */
/* ncsh_input_prompt_size
 * get the prompt size accounting for prompt length, user length, and cwd length
 * Returns: length of the prompt
 */
[[nodiscard]]
size_t ncsh_input_prompt_size(const size_t user_len, const size_t dir_len)
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

/* ncsh_input_prompt_short_directory_get
 * gets a shortened version of the cwd, the last 2 directories in the cwd.
 * i.e. /home/alex/dir becomes /alex/dir
 * Returns: length of the shortened cwd
 */
[[nodiscard]]
size_t ncsh_input_prompt_short_directory_get(const char* const cwd, char* output)
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

/* ncsh_input_prompt_short_directory_print
 * Gets and prints the shortened directory prompt when it is enabled at compile time via ncsh_configurables.h option.
 * Handles whether User is included in prompt or not, based on
 * Returns: EXIT_FAILURE or EXIT_SUCCESS
 */
[[nodiscard]]
int_fast32_t ncsh_input_prompt_short_directory_print(struct ncsh_Input* const restrict input)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    // size_t dir_len = ncsh_input_prompt_short_directory_get(cwd, directory);
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value, directory);
    // input->prompt_len = ncsh_input_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, directory);
    input->prompt_len = ncsh_input_prompt_size(0, dir_len);
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
int_fast32_t ncsh_input_prompt_directory_print(struct ncsh_Input* const restrict input)
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
    input->prompt_len = ncsh_input_prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, cwd);
    input->prompt_len = ncsh_input_prompt_size(0, dir_len);
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
int_fast32_t ncsh_input_prompt_no_directory_print(struct ncsh_Input* const restrict input)
{
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value);
    // input->prompt_len = ncsh_input_prompt_size(input->user.length, 0);
    (void)ncsh_input_prompt_size(input->user.length, 0);
#else
    printf(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);
    // input->prompt_len = ncsh_input_prompt_size(0, 0);
    (void)ncsh_input_prompt_size(0, 0);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif

/* ncsh_input_prompt
 * Prints the prompt based on the current prompt compile-time settings.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int_fast32_t ncsh_input_prompt(struct ncsh_Input* const restrict input)
{
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    return ncsh_input_prompt_short_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    return ncsh_input_prompt_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    return ncsh_input_prompt_no_directory_print(input);
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */
}

// IO
[[nodiscard]]
int_fast32_t ncsh_input_autocomplete(struct ncsh_Input* const restrict input,
                                        struct ncsh_Arena* scratch_arena)
{
    if (input->buffer[0] == '\0' || input->buffer[input->pos] != '\0') {
        return EXIT_SUCCESS;
    }
    else if (input->buffer[0] < 32) { // exclude control characters from autocomplete
	/*if (ncsh_input_line_reset(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }*/
        // memset(input->buffer, '\0', input->max_pos);
        memset(input->buffer, '\0', input->pos);
        input->pos = 0;
        // input->max_pos = 0;
        return EXIT_SUCCESS;
    }

    uint_fast8_t autocompletions_matches_count = ncsh_autocompletions_first(
        input->buffer, input->pos + 1, input->current_autocompletion,
        input->autocompletions_tree, *scratch_arena);

    if (!autocompletions_matches_count) {
        /*if (ncsh_input_line_reset(input) != EXIT_SUCCESS) {
            return EXIT_FAILURE;
        }*/
        input->current_autocompletion[0] = '\0';
	input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }

    input->current_autocompletion_len = strlen(input->current_autocompletion);
    /*if (input->current_y == 0 && input->prompt_len + (size_t)input->lines_x[input->lines_y] +
        input->current_autocompletion_len > (size_t)input->terminal_size.x) {
	input->current_autocompletion[0] = '\0';
	input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }
    else if ((size_t)input->lines_x[input->lines_y] + input->current_autocompletion_len > (size_t)input->terminal_size.x) {
	input->current_autocompletion[0] = '\0';
	input->current_autocompletion_len = 0;
        return EXIT_SUCCESS;
    }*/

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
    ncsh_terminal_move_left(input->current_autocompletion_len);
    fflush(stdout);
    return EXIT_SUCCESS;
}

/* ncsh_input_autocomplete_select
 * render the current autocompletion.
 */
[[nodiscard]]
int_fast32_t ncsh_input_autocomplete_select(struct ncsh_Input* const restrict input)
{
    printf("%s", input->current_autocompletion);
    for (uint_fast32_t i = 0; input->current_autocompletion[i] != '\0'; i++) {
        input->buffer[input->pos] = input->current_autocompletion[i];
        ++input->pos;
    }
    input->buffer[input->pos] = '\0';

    /*if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }*/

    fflush(stdout);
    input->current_autocompletion[0] = '\0';
    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_input_move_cursor_right(struct ncsh_Input* const restrict input)
{
    if (input->pos == NCSH_MAX_INPUT - 1 || (!input->buffer[input->pos] && !input->buffer[input->pos + 1])) {
        return EXIT_SUCCESS;
    }

    ncsh_write_literal(MOVE_CURSOR_RIGHT);

    ++input->pos;
    /*if (input->pos > input->max_pos) {
        input->max_pos = input->pos;
    }*/

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_input_history_up(struct ncsh_Input* const restrict input)
{
    input->history_entry = ncsh_history_get(input->history_position, &input->history);
    if (input->history_entry.length > 0) {
        ++input->history_position;
        ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

        input->pos = input->history_entry.length - 1;
        // input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        ncsh_write(input->buffer, input->pos);
        // ncsh_input_adjust_line_if_needed(input);
    }

    return EXIT_SUCCESS;
}

[[nodiscard]]
int_fast32_t ncsh_input_history_down(struct ncsh_Input* const restrict input)
{
    input->history_entry = ncsh_history_get(input->history_position - 2, &input->history);

    ncsh_write_literal(RESTORE_CURSOR_POSITION ERASE_CURRENT_LINE);

    if (input->history_entry.length > 0) {
        --input->history_position;
        input->pos = input->history_entry.length - 1;
        // input->max_pos = input->history_entry.length - 1;
        memcpy(input->buffer, input->history_entry.value, input->pos);

        ncsh_write(input->buffer, input->pos);
    }
    else {
        input->buffer[0] = '\0';
        input->pos = 0;
        // input->max_pos = 0;
    }

    // ncsh_input_adjust_line_if_needed(input);

    return EXIT_SUCCESS;
}

[[nodiscard]]
char ncsh_input_read(void)
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
int_fast32_t ncsh_input_tab_autocomplete(struct ncsh_Input* const restrict input,
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
        if ((character = ncsh_input_read()) == EXIT_IO_FAILURE) {
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

/* ncsh_input_init
 * Allocates memory that lives for the lifetime of the shell and is used by readline to process user input.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
int_fast32_t ncsh_input_init(struct ncsh_Config* const restrict config,
                                struct ncsh_Input* const restrict input,
                                struct ncsh_Arena* const arena)
{
    input->user.value = getenv("USER");
    input->user.length = strlen(input->user.value) + 1;
    input->buffer = arena_malloc(arena, NCSH_MAX_INPUT, char);

    enum eskilib_Result result;
    if ((result = ncsh_history_init(config->config_location, &input->history, arena)) != E_SUCCESS) {
        perror(RED "ncsh: Error when allocating data for and setting up history" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    input->current_autocompletion = arena_malloc(arena, NCSH_MAX_INPUT, char);
    input->autocompletions_tree = ncsh_autocompletions_alloc(arena);

    ncsh_autocompletions_add_multiple(input->history.entries, input->history.count, input->autocompletions_tree, arena);

    return EXIT_SUCCESS;
}

/* ncsh_input
 * Read user input while supporting different operations like backspace, delete, history, autocompletions, home/end, and other inputs.
 * Returns: exit status, EXIT_SUCCESS, EXIT_FAILURE, or value in ncsh_defines.h (EXIT_...)
 */
[[nodiscard]]
int_fast32_t ncsh_input(struct ncsh_Input* const restrict input,
                           struct ncsh_Arena scratch_arena)
{
    (void)scratch_arena;

    char directory[PATH_MAX] = {0};
    size_t dir_len = 0;
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    dir_len = ncsh_input_prompt_short_directory_get(cwd, directory) - 1;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    if (!getcwd(directory, sizeof(directory))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    dir_len = strlen(directory);
#endif /* ifdef  NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */


#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NONE
    char prompt[PATH_MAX] = ncsh_CYAN;
    constexpr size_t start_pos = sizeof(ncsh_CYAN) - 1;
    size_t pos = start_pos;
#else
    char prompt[PATH_MAX] = ncsh_GREEN;
    constexpr size_t start_pos = sizeof(ncsh_GREEN) - 1;
    size_t pos = start_pos;
    memcpy(prompt + pos, input->user.value, input->user.length);
    pos += input->user.length - 1;
#   if NCSH_PROMPT_DIRECTORY != NCSH_DIRECTORY_NONE
        memcpy(prompt + pos, " " ncsh_CYAN, sizeof(ncsh_CYAN));
        pos += sizeof(ncsh_CYAN);
#   endif
#endif

    if (dir_len > 0) {
	memcpy(prompt + pos, directory, dir_len);
    	pos += dir_len;
    }
    memcpy(prompt + pos, WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, sizeof(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING));

    readline_input rl_input = {0};
    rl_input.prompt = prompt;

    input->buffer = ncsh_readline(&rl_input);

    if (!input->buffer) {
        return EXIT_SUCCESS_END;
    }

    input->pos = strlen(input->buffer) + 1;
    return EXIT_SUCCESS;
}
