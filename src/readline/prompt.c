/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "../configurables.h"
#include "../defines.h"
#include "ncreadline.h"
#include "terminal.h"

/* Prompt */
/* prompt_size
 * get the prompt size accounting for prompt length, user length, and cwd length
 * Returns: length of the prompt
 */
[[nodiscard]]
size_t prompt_size(size_t user_len, size_t dir_len)
{
    // shell prompt format:
    // {user} {directory} {symbol} {buffer}
    // user, directory include null termination, use as space for len
    //     {user}{space (\0)}      {directory}{space (\0) excluded by -1}     {>}  {space}     {buffer}
    return user_len == 0 && dir_len == 0 ? NCSH_PROMPT_ENDING_STRING_LENGTH
                                         : user_len + dir_len - 1 + NCSH_PROMPT_ENDING_STRING_LENGTH;
}

/* This section is included at compile time if the prompt directory setting is set to use shortened directory. */
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT

/* prompt_short_directory_get
 * gets a shortened version of the cwd, the last 2 directories in the cwd.
 * i.e. /home/alex/dir becomes /alex/dir
 * Returns: length of the shortened cwd
 */
[[nodiscard]]
size_t prompt_short_directory_get(char* rst cwd, char* rst output)
{
    assert(cwd);

    size_t i = 1;
    size_t last_slash_pos = 0;
    size_t second_to_last_slash_pos = 0;

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
    return i - last_slash_pos + 2;                                    // null termination included in len
}

/* prompt_short_directory_print
 * Gets and prints the shortened directory prompt when it is enabled at compile time via configurables.h option.
 * Handles whether User is included in prompt or not, based on
 * Returns: EXIT_FAILURE or EXIT_SUCCESS
 */
[[nodiscard]]
int prompt_short_directory_print(Input* rst input)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    size_t dir_len = prompt_short_directory_get(cwd, directory);
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s"
                      " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING,
           input->user.value, directory);
    input->prompt_len = prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, directory);
    input->prompt_len = prompt_size(0, dir_len);
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
int prompt_directory_print(Input* rst input)
{
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return EXIT_FAILURE;
    }
    size_t dir_len = strlen(cwd) + 1;

#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s"
                      " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING,
           input->user.value, cwd);
    input->prompt_len = prompt_size(input->user.length, dir_len);
#else
    printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, cwd);
    input->prompt_len = prompt_size(0, dir_len);
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
int prompt_no_directory_print(Input* rst input)
{
#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL
    printf(ncsh_GREEN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value);
    input->prompt_len = prompt_size(input->user.length, 0);
#else
    printf(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);
    input->prompt_len = prompt_size(0, 0);
#endif /* if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NORMAL */

    fflush(stdout);
    // save cursor position so we can reset cursor when loading history entries
    ncsh_write_literal(SAVE_CURSOR_POSITION);
    return EXIT_SUCCESS;
}
#endif

/* prompt
 * Prints the prompt based on the current prompt compile-time settings.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int prompt(Input* rst input)
{
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    return prompt_short_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    return prompt_directory_print(input);
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NONE
    return prompt_no_directory_print(input);
#endif /* if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */
}

/* prompt_if_needed
 * Prints the prompt if input.reprint_prompt is true.
 * Calls prompt.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int prompt_if_needed(Input* rst input)
{
    if (input->reprint_prompt == true) {
        if (prompt(input) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        input->lines_y = 0;
        input->history_position = 0;
        input->reprint_prompt = false;
    }
    return EXIT_SUCCESS;
}
