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
#include "ncsh_input.h"

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
#endif /* NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */

[[nodiscard]]
char* ncsh_input_prompt(struct ncsh_String* user,
                        struct ncsh_Arena* const scratch_arena)
{
    char directory[PATH_MAX] = {0};
    size_t dir_len = 0;
#if NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT
    char cwd[PATH_MAX] = {0};
    if (!getcwd(cwd, sizeof(cwd))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return NULL;
    }
    dir_len = ncsh_input_prompt_short_directory_get(cwd, directory) - 1;
#elif NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_NORMAL
    if (!getcwd(directory, sizeof(directory))) {
        perror(RED "ncsh: Error when getting current directory" RESET);
        fflush(stderr);
        return NULL;
    }
    dir_len = strlen(directory);
#endif /* ifdef  NCSH_PROMPT_DIRECTORY == NCSH_DIRECTORY_SHORT */


#if NCSH_PROMPT_SHOW_USER == NCSH_SHOW_USER_NONE
    (void)user; // prevent compiler warnings
    char prompt[PATH_MAX] = ncsh_CYAN;
    constexpr size_t start_pos = sizeof(ncsh_CYAN) - 1;
    size_t pos = start_pos;
#else
    char prompt[PATH_MAX] = ncsh_GREEN;
    constexpr size_t start_pos = sizeof(ncsh_GREEN) - 1;
    size_t pos = start_pos;
    memcpy(prompt + pos, user->value, user->length);
    pos += user->length - 1;
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
    pos += sizeof(WHITE_BRIGHT NCSH_PROMPT_ENDING_STRING);

    char* out = arena_malloc(scratch_arena, pos, char);
    memcpy(out, prompt, pos);
    return out;
}

[[nodiscard]]
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

[[nodiscard]]
int_fast32_t ncsh_input(struct ncsh_Input* const restrict input,
                           struct ncsh_Arena scratch_arena)
{
    readline_input rl_input = {0};
    rl_input.prompt = ncsh_input_prompt(&input->user, &scratch_arena);
    if (!rl_input.prompt) {
	return EXIT_FAILURE;
    }
    rl_input.tree = input->autocompletions_tree;
    rl_input.scratch_arena = scratch_arena;

    input->buffer = ncsh_readline(&rl_input);
    if (!input->buffer) {
        return EXIT_SUCCESS_END;
    }
    input->pos = strlen(input->buffer) + 1;

    return EXIT_SUCCESS;
}
