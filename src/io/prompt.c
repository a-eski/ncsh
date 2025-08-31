/* Copyright ncsh (C) by Alex Eski 2025 */

#include <assert.h>
#include <linux/limits.h>
#include <stddef.h>
#include <unistd.h>

#include "../defines.h" // used for macros
#include "prompt.h"
#include "../ttyio/ttyio.h"

#define USER_COLOR 147
#define DIRECTORY_COLOR 10

static Prompt_Data prompt_data;

/* prompt_short_directory_get
 * gets a shortened version of the cwd, the last 2 directories in the cwd.
 * i.e. /home/alex/dir becomes /alex/dir
 * Returns: length of the shortened cwd
 */
[[nodiscard]]
size_t prompt_short_directory_get(char* restrict cwd, char* restrict output)
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
    return i - last_slash_pos + 1;                                    // null termination not included in len
}

/* prompt_short_directory_print
 * Gets and prints the shortened directory prompt when it is enabled at compile time via configurables.h option.
 * Handles whether User is included in prompt or not, based on
 * Returns: EXIT_FAILURE or EXIT_SUCCESS
 */
[[nodiscard]]
int prompt_short_directory_print(Input* restrict input)
{
    char cwd[PATH_MAX] = {0};
    char directory[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        tty_perror("ncsh: Error when getting current directory");
        return EXIT_FAILURE;
    }

    size_t dir_len = prompt_short_directory_get(cwd, directory);
    int printed = 0;
    if (prompt_data.show_user) {
        tty_color_set(USER_COLOR);
        printed += tty_write(input->user->value, input->user->length);
        printed += tty_putc(' ');
        tty_color_set(DIRECTORY_COLOR);
        printed += tty_write(directory, dir_len);
        tty_send(&tcaps.color_reset);
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);

        /*printf(ncsh_GREEN "%s"
                      " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING,
           input->user.value, directory);*/
        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }
    else {
        tty_color_set(DIRECTORY_COLOR);
        printed += tty_write(directory, dir_len);
        tty_send(&tcaps.color_reset);
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);
        // printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, directory);
        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }

    // save cursor position so we can reset cursor when loading history entries
    tty_send(&tcaps.cursor_save);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int prompt_directory_print(Input* restrict input)
{
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        tty_perror("ncsh: Error when getting current directory");
        return EXIT_FAILURE;
    }
    int printed = 0;
    if (prompt_data.show_user) {
        tty_color_set(USER_COLOR);
        printed += tty_write(input->user->value, input->user->length);
        printed += tty_putc(' ');
        tty_color_set(DIRECTORY_COLOR);
        printed += tty_print("%s", cwd);
        tty_send(&tcaps.color_reset);
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);

        /*printf(ncsh_GREEN "%s"
                          " " ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING,
               input->user.value, cwd);*/
        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }
    else {
        tty_color_set(DIRECTORY_COLOR);
        printed += tty_print("%s", cwd);
        tty_send(&tcaps.color_reset);
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);
        // printf(ncsh_CYAN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, cwd);
        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }


    // save cursor position so we can reset cursor when loading history entries
    tty_send(&tcaps.cursor_save);
    return EXIT_SUCCESS;
}

[[nodiscard]]
int prompt_no_directory_print(Input* restrict input)
{
    int printed = 0;
    if (prompt_data.show_user) {
        tty_color_set(USER_COLOR);
        printed += tty_write(input->user->value, input->user->length);
        tty_send(&tcaps.color_reset);
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);
        // printf(ncsh_GREEN "%s" WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING, input->user.value);

        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }
    else {
        printed += tty_print(NCSH_PROMPT_ENDING_STRING);

        // printf(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);
        assert(printed > 0);
        input->prompt_len = (size_t)printed;
    }

    // save cursor position so we can reset cursor when loading history entries
    tty_send(&tcaps.cursor_save);
    return EXIT_SUCCESS;
}

/* prompt
 * Prints the prompt based on the current prompt compile-time settings.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int prompt(Input* restrict input)
{
    switch (prompt_data.dir_type) {
        case DIR_SHORT:
            return prompt_short_directory_print(input);
        case DIR_NORMAL:
            return prompt_directory_print(input);
        case DIR_NONE:
            return prompt_no_directory_print(input);
        default:
            unreachable();
            return 0;
    }
}

/* prompt_if_needed
 * Prints the prompt if input.reprint_prompt is true.
 * Calls prompt.
 * Returns: the length of the prompt.
 */
[[nodiscard]]
int prompt_if_needed(Input* restrict input)
{
    if (input->reprint_prompt == true) {
        if (prompt(input) == EXIT_FAILURE) {
            tty_send(&tcaps.cursor_show);
            return EXIT_FAILURE;
        }
        tty_send(&tcaps.cursor_show);
        input->lines_y = 0;
        input->history_position = 0;
        input->reprint_prompt = false;
    }
    return EXIT_SUCCESS;
}

void prompt_init(bool showUser, enum Dir_Type dir_type)
{
    prompt_data.show_user = showUser;
    prompt_data.dir_type = dir_type;
}
